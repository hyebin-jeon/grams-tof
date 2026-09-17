import paho.mqtt.client as mqtt
import mysql.connector
import struct
import os
import sys

# --- Environment Configuration ---
#db_host = os.getenv("TOF_MYSQL_HOST", "127.0.0.1")
#db_user = os.getenv("TOF_MYSQL_USER")
#db_pass = os.getenv("TOF_MYSQL_PASS")
#db_name = os.getenv("TOF_MYSQL_DB")
db_host = os.getenv("PGRAMS_MYSQL_HOST", "localhost")
db_port = int(os.getenv("PGRAMS_MYSQL_PORT", 3306))
db_user = os.getenv("PGRAMS_MYSQL_USER")
db_pass = os.getenv("PGRAMS_MYSQL_PASSWD")
db_name = os.getenv("PGRAMS_MYSQL_DB")

mq_host = os.getenv("PGRAMS_MOSQUITTO_HOST", "localhost")
mq_port = int(os.getenv("PGRAMS_MOSQUITTO_PORT", 1883))
mq_topic = os.getenv("PGRAMS_MOSQUITTO_TOPIC", "TOF_ground_telemetry")

if not all([db_user, db_pass, db_name]):
    print("Error: MySQL Environment variables not set!")
    sys.exit(1)

LOG_LEVEL_MAP = {
    0: "TRACE",
    1: "DEBUG",
    2: "INFO",
    3: "NOTICE",
    4: "WARN",
    5: "ERROR",
    6: "CRITICAL"
}

# --- Database Operations ---
def insert_hist_to_db(run_num, hname, bins, h_type, nx, xmin, xmax, ny, ymin, ymax):
    try:
        conn = mysql.connector.connect(
            host=db_host, user=db_user, password=db_pass, database=db_name
        )
        cursor = conn.cursor()

        data_to_insert = []
        for idx, val in enumerate(bins):
            float_val = struct.unpack('f', struct.pack('I', val))[0]
            data_to_insert.append((
                run_num, hname, idx, float_val,
                h_type, nx, xmin, xmax, ny, ymin, ymax
            ))

        query = """
            INSERT INTO tof_monitor (
                run_number, hname, bin_index, bin_content,
                hist_type, n_bins_x, x_min, x_max, n_bins_y, y_min, y_max
            ) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """
        cursor.executemany(query, data_to_insert)
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Database Error (Hist): {e}")

def insert_log_to_db(run_number, timestamp_ms, level, level_str, component, message):
    try:
        conn = mysql.connector.connect(
            host=db_host, port=db_port, user=db_user, password=db_pass, database=db_name
        )
        cursor = conn.cursor()

        query = """
            INSERT INTO tof_logs (
                run_number, timestamp_ms, level, level_str, component, message
            ) VALUES (%s, %s, %s, %s, %s, %s)
        """
        cursor.execute(query, (run_number, timestamp_ms, level, level_str, component, message))
        conn.commit()
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Database Error (Log): {e}")

# --- Handlers ---
def handle_log_stream(payload):
    if len(payload) < 50:
        return

    argv_data = payload[8:-6]
    num_words = len(argv_data) // 4
    if num_words < 9:
        return

    argv = struct.unpack(f'>{num_words}I', argv_data[:num_words * 4])

    run_number = argv[0]
    timestamp_ms = (argv[1] << 32) | argv[2]
    level = argv[3] & 0xFF
    msg_len = argv[4]

    comp_raw = struct.pack('>4I', argv[5], argv[6], argv[7], argv[8])
    comp_bytes = b"".join([comp_raw[i:i+4][::-1] for i in range(0, 16, 4)])
    component = comp_bytes.decode('utf-8', errors='ignore').rstrip('\x00')

    if msg_len > 0 and len(argv) >= 9 + ((msg_len + 3) // 4):
        msg_words = argv[9 : 9 + ((msg_len + 3) // 4)]
        msg_raw = struct.pack(f'>{len(msg_words)}I', *msg_words)
        
        fixed_msg_bytes = b"".join([msg_raw[i:i+4][::-1] for i in range(0, len(msg_raw), 4)])
        message = fixed_msg_bytes[:msg_len].decode('utf-8', errors='ignore')
    else:
        message = ""

    level_str = LOG_LEVEL_MAP.get(level, f"LOG({level})")

    print(f"   -> Log Decoded: [{level_str}] [{component}] {message}")
    insert_log_to_db(run_number, timestamp_ms, level, level_str, component, message)

def handle_histogram(payload):
    m_header = payload[8:56]
    unpacked = struct.unpack('>I16sIIffIff', m_header)

    run_num  = unpacked[0]
    h_type   = unpacked[2]
    n_bins_x = unpacked[3]
    x_min    = unpacked[4]
    x_max    = unpacked[5]
    n_bins_y = unpacked[6]
    y_min    = unpacked[7]
    y_max    = unpacked[8]

    if h_type != 2:
        y_min, y_max = 0.0, 0.0

    raw_name_bytes = unpacked[1]
    fixed_name_parts = [raw_name_bytes[i:i+4][::-1] for i in range(0, 16, 4)]
    hname = b"".join(fixed_name_parts).decode('utf-8').strip('\x00')

    if h_type == 3:
        n_bins = n_bins_x * 3
    elif h_type == 2:
        n_bins = (n_bins_x + 2) * (n_bins_y + 2)
    else:
        n_bins = (n_bins_x + 2)

    bin_start = 56
    actual_available = (len(payload) - bin_start) // 4
    n_bins_to_read = min(n_bins, actual_available)

    bin_values = struct.unpack(f'>{n_bins_to_read}I', payload[bin_start:bin_start + (n_bins_to_read * 4)])

    print(f"   -> Hist Decoded {hname}: Run {run_num}, Type {h_type}")
    insert_hist_to_db(run_num, hname, bin_values, h_type, n_bins_x, x_min, x_max, n_bins_y, y_min, y_max)

# --- MQTT Callback ---
def on_message(client, userdata, msg):
    payload = msg.payload
    if len(payload) < 8:
        return

    try:
        h1, h2, code = struct.unpack('>HHH', payload[0:6])
        
        if code == 0x5401: # LOGGER_DATA_STREAM
            handle_log_stream(payload)
        else:             # Histogram payload
            handle_histogram(payload)

    except Exception as e:
        print(f"Decoding Error: {e}")

# --- Main Run ---
client = mqtt.Client()
client.on_message = on_message

print(f"Connecting to MQTT Broker: {mq_host}:{mq_port}...")
client.connect(mq_host, mq_port)
client.subscribe(mq_topic)

print("TOF Bridge is running...")
client.loop_forever()
