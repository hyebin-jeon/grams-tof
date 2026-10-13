import paho.mqtt.client as mqtt
import mysql.connector
import struct
import os
import sys

# --- Environment Configuration ---
db_host = os.getenv("PGRAMS_MYSQL_HOST", "localhost")
db_port = 3306
db_user = os.getenv("PGRAMS_MYSQL_USER")
db_pass = os.getenv("PGRAMS_MYSQL_PASSWD")
db_name = "grams_tof"

mq_host   = os.getenv("PGRAMS_MOSQUITTO_HOST",  "localhost")
mq_port   = int(os.getenv("PGRAMS_MOSQUITTO_PORT", 1883))
mq_user   = os.getenv("PGRAMS_MOSQUITTO_USER")
mq_passwd = os.getenv("PGRAMS_MOSQUITTO_PASSWD")
mq_topic  = os.getenv("PGRAMS_MOSQUITTO_TOPIC", "TOF_ground_telemetry")

if not all([db_user, db_pass, db_name]):
    print("Error: MySQL Environment variables not set!")
    sys.exit(1)

# data_type sub-codes for 0x5400
# Must match MonitorCodec::DataType enum
# packet.argv[5] = payload[8 + 5*4] = payload[28:32]
DATA_TYPE_HIST      = 1  # DataType::TH1F / TH2F / TProfile
DATA_TYPE_GRAPH     = 4  # DataType::TGraph
DATA_TYPE_PARAMETER = 5  # DataType::TParameter

LOG_LEVEL_MAP = {
    0: "TRACE", 1: "DEBUG", 2: "INFO",  3: "NOTICE",
    4: "WARN",  5: "ERROR", 6: "CRITICAL"
}

# ─────────────────────────────────────────────
#  DB helpers
# ─────────────────────────────────────────────

def _connect():
    return mysql.connector.connect(
        host=db_host, port=db_port,
        user=db_user, password=db_pass, database=db_name
    )

def insert_hist_to_db(run_num, hname, bins, h_type, nx, xmin, xmax, ny, ymin, ymax):
    try:
        conn   = _connect()
        cursor = conn.cursor()
        data_to_insert = [
            (run_num, hname, idx,
             struct.unpack('f', struct.pack('I', val))[0],
             h_type, nx, xmin, xmax, ny, ymin, ymax)
            for idx, val in enumerate(bins)
        ]
        cursor.executemany("""
            INSERT INTO tof_monitor (
                run_number, hname, bin_index, bin_content,
                hist_type, n_bins_x, x_min, x_max, n_bins_y, y_min, y_max
            ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """, data_to_insert)
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Database Error (Hist): {e}")

def insert_graph_to_db(run_num, gname, points_xy):
    try:
        conn   = _connect()
        cursor = conn.cursor()
        cursor.executemany("""
            INSERT INTO tof_graphs (run_number, gname, point_index, x_val, y_val)
            VALUES (%s, %s, %s, %s, %s)
        """, [(run_num, gname, idx, x, y) for idx, (x, y) in enumerate(points_xy)])
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Database Error (Graph): {e}")

def insert_parameter_to_db(run_num, pname, value):
    try:
        conn   = _connect()
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO tof_parameters (run_number, pname, value)
            VALUES (%s, %s, %s)
        """, (run_num, pname, value))
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Database Error (Parameter): {e}")

def insert_log_to_db(run_number, timestamp_ms, level, level_str, component, message):
    try:
        conn   = _connect()
        cursor = conn.cursor()
        cursor.execute("""
            INSERT INTO tof_logs (
                run_number, timestamp_ms, level, level_str, component, message
            ) VALUES (%s, %s, %s, %s, %s, %s)
        """, (run_number, timestamp_ms, level, level_str, component, message))
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Database Error (Log): {e}")

# ─────────────────────────────────────────────
#  Packet handlers
# ─────────────────────────────────────────────

def _decode_name(raw16: bytes) -> str:
    """Decode a 16-byte big-endian word-swapped name field."""
    parts = [raw16[i:i+4][::-1] for i in range(0, 16, 4)]
    return b"".join(parts).decode('utf-8', errors='ignore').strip('\x00')

def handle_log_stream(payload):
    if len(payload) < 50:
        return

    argv_data = payload[8:-6]
    num_words = len(argv_data) // 4
    if num_words < 9:
        return

    argv = struct.unpack(f'>{num_words}I', argv_data[:num_words * 4])

    run_number   = argv[0]
    timestamp_ms = (argv[1] << 32) | argv[2]
    level        = argv[3] & 0xFF
    msg_len      = argv[4]

    comp_raw  = struct.pack('>4I', argv[5], argv[6], argv[7], argv[8])
    component = _decode_name(comp_raw)

    if msg_len > 0 and len(argv) >= 9 + ((msg_len + 3) // 4):
        msg_words = argv[9 : 9 + ((msg_len + 3) // 4)]
        msg_raw   = struct.pack(f'>{len(msg_words)}I', *msg_words)
        fixed     = b"".join([msg_raw[i:i+4][::-1] for i in range(0, len(msg_raw), 4)])
        message   = fixed[:msg_len].decode('utf-8', errors='ignore')
    else:
        message = ""

    level_str = LOG_LEVEL_MAP.get(level, f"LOG({level})")
    print(f"   -> Log Decoded: [{level_str}] [{component}] {message}")
    insert_log_to_db(run_number, timestamp_ms, level, level_str, component, message)

def handle_monitor(payload):
    """
    Common handler for 0x5400 packets.
    Dispatches to Hist / Graph / Parameter based on MonitorCodec::DataType (argv[5]).

    argv layout (payload[8:] is the argv region):
      argv[0]   = run_number          -> payload[8:12]
      argv[1-4] = name (16 bytes)     -> payload[12:28]
      argv[5]   = data_type           -> payload[28:32]
        1 = TH1F / TH2F / TProfile
        4 = TGraph
        5 = TParameter<double>
    """
    if len(payload) < 32:
        return

    data_type, = struct.unpack('>I', payload[28:32])

    if data_type == DATA_TYPE_HIST:
        _handle_histogram(payload)
    elif data_type == DATA_TYPE_GRAPH:
        _handle_graph(payload)
    elif data_type == DATA_TYPE_PARAMETER:
        _handle_parameter(payload)
    else:
        print(f"   -> Unknown monitor data_type: {data_type}")

def _handle_histogram(payload):
    # argv layout:
    #   [0]    run_number   -> payload[8:12]
    #   [1-4]  hname        -> payload[12:28]
    #   [5]    hist_type    -> payload[28:32]  (1=TH1F, 2=TH2F, 3=TProfile)
    #   [6]    n_bins_x     -> payload[32:36]
    #   [7]    x_min        -> payload[36:40]
    #   [8]    x_max        -> payload[40:44]
    #   [9]    n_bins_y     -> payload[44:48]
    #   [10]   y_min        -> payload[48:52]
    #   [11]   y_max        -> payload[52:56]
    #   [12+]  bins         -> payload[56:]
    if len(payload) < 56:
        return

    run_num,  = struct.unpack('>I', payload[8:12])
    hname     = _decode_name(payload[12:28])
    h_type,   = struct.unpack('>I', payload[28:32])
    n_bins_x, = struct.unpack('>I', payload[32:36])
    x_min,    = struct.unpack('>f', payload[36:40])
    x_max,    = struct.unpack('>f', payload[40:44])
    n_bins_y, = struct.unpack('>I', payload[44:48])
    y_min     = struct.unpack('>f', payload[48:52])[0] if h_type == 2 else 0.0
    y_max     = struct.unpack('>f', payload[52:56])[0] if h_type == 2 else 0.0

    # DataType enum: TH1F=1, TH2F=2, TProfile=3
    if h_type == 3:    # TProfile: mean/rms/entries triplets, no overflow bins
        n_bins = n_bins_x * 3
    elif h_type == 2:  # TH2F: includes underflow/overflow
        n_bins = (n_bins_x + 2) * (n_bins_y + 2)
    else:              # TH1F: includes underflow/overflow
        n_bins = (n_bins_x + 2)

    bin_start        = 56
    n_bins_to_read   = min(n_bins, (len(payload) - bin_start) // 4)
    bin_values = struct.unpack(
        f'>{n_bins_to_read}I',
        payload[bin_start : bin_start + n_bins_to_read * 4]
    )

    print(f"   -> Hist Decoded {hname}: Run {run_num}, Type {h_type}")
    insert_hist_to_db(run_num, hname, bin_values, h_type, n_bins_x, x_min, x_max, n_bins_y, y_min, y_max)

def _handle_graph(payload):
    # argv layout:
    #   [0]    run_number   -> payload[8:12]
    #   [1-4]  gname        -> payload[12:28]
    #   [5]    data_type=4  -> payload[28:32]  (already verified by handle_monitor)
    #   [6]    n_points     -> payload[32:36]
    #   [7+]   X/Y pairs    -> payload[36:]
    if len(payload) < 36:
        return

    run_num,  = struct.unpack('>I', payload[8:12])
    gname     = _decode_name(payload[12:28])
    n_points, = struct.unpack('>I', payload[32:36])

    pt_start      = 36
    # Each point: X_hi(I) X_lo(I) Y_hi(I) Y_lo(I) = 16 bytes
    n_pts_to_read = min(n_points, (len(payload) - pt_start) // 16)

    points_xy = []
    for i in range(n_pts_to_read):
        base = pt_start + i * 16
        x_hi, x_lo, y_hi, y_lo = struct.unpack('>IIII', payload[base : base + 16])
        x_bits = (x_hi << 32) | x_lo
        y_bits = (y_hi << 32) | y_lo
        x, = struct.unpack('>d', x_bits.to_bytes(8, 'big'))
        y, = struct.unpack('>d', y_bits.to_bytes(8, 'big'))
        points_xy.append((x, y))

    print(f"   -> Graph Decoded {gname}: Run {run_num}, Points {n_pts_to_read}")
    insert_graph_to_db(run_num, gname, points_xy)

def _handle_parameter(payload):
    # argv layout:
    #   [0]    run_number   -> payload[8:12]
    #   [1-4]  pname        -> payload[12:28]
    #   [5]    data_type=5  -> payload[28:32]  (already verified by handle_monitor)
    #   [6]    value hi     -> payload[32:36]
    #   [7]    value lo     -> payload[36:40]
    if len(payload) < 40:
        return

    run_num,   = struct.unpack('>I', payload[8:12])
    pname      = _decode_name(payload[12:28])
    val_hi,    = struct.unpack('>I', payload[32:36])
    val_lo,    = struct.unpack('>I', payload[36:40])
    bits       = (val_hi << 32) | val_lo
    value,     = struct.unpack('>d', bits.to_bytes(8, 'big'))

    print(f"   -> Parameter Decoded {pname}: Run {run_num}, Value {value:.6g}")
    insert_parameter_to_db(run_num, pname, value)

# ─────────────────────────────────────────────
#  MQTT callback
# ─────────────────────────────────────────────

PACKET_HANDLERS = {
    0x5400: handle_monitor,     # Hist / Graph / Parameter (dispatched by data_type)
    0x5401: handle_log_stream,  # Logger data stream
}

def on_message(client, userdata, msg):
    print(f"MQTT RECEIVED: topic={msg.topic}, payload_size={len(msg.payload)}")

    payload = msg.payload
    if len(payload) < 8:
        return

    try:
        h1, h2, code = struct.unpack('>HHH', payload[0:6])
        print(f"MQTT header: h1=0x{h1:04x}, h2=0x{h2:04x}, code=0x{code:04x}")

        handler = PACKET_HANDLERS.get(code)
        if handler:
            handler(payload)
        else:
            print(f"Unknown TOF packet: code=0x{code:04x}, size={len(payload)}")

    except Exception as e:
        print(f"Decoding Error: {e}")

# ─────────────────────────────────────────────
#  Main
# ─────────────────────────────────────────────

client = mqtt.Client()

if mq_user and mq_passwd:
    client.username_pw_set(mq_user, mq_passwd)

client.on_message = on_message

print(f"Connecting to MQTT Broker: {mq_host}:{mq_port}...")
client.connect(mq_host, mq_port)
client.subscribe(mq_topic)

print("TOF Bridge is running...")
client.loop_forever()
