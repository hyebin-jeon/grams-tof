import paho.mqtt.client as mqtt
import mysql.connector
import json
import struct
import os

# Load configuration
config_path = os.path.join(os.path.dirname(__file__), '../config/bridge_config.json')
with open(config_path) as f:
    conf = json.load(f)

def insert_to_db(run_num, hname, bins, h_type, nx, xmin, xmax, ny, ymin, ymax):
    try:
        conn = mysql.connector.connect(**conf['mysql'])
        cursor = conn.cursor()
        
        # Prepare 'Long' format data with full metadata context
        data_to_insert = []
        for idx, val in enumerate(bins):
            # Convert raw uint32 bits back to float
            float_val = struct.unpack('f', struct.pack('I', val))[0]
            
            # Pack all 11 fields required by your new schema
            data_to_insert.append((
                run_num, hname, idx, float_val, 
                h_type, nx, xmin, xmax, ny, ymin, ymax
            ))

        # Explicitly list all columns to ensure data integrity
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
        print(f"Database Error: {e}")

def on_message(client, userdata, msg):
    payload = msg.payload
    print(f"Processing {len(payload)} bytes from {msg.topic}")
    
    try:
        # 1. Verify Headers (Bytes 0-4) 
        h1, h2 = struct.unpack('>HH', payload[0:4])

        # 2. Extract MonitorData Header (48 bytes starting at offset 8)
        # Format: I (run), 16s (name), I (type), I (nx), f (xmin), f (xmax), I (ny), f (ymin), f (ymax)
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
            y_min = 0.0
            y_max = 0.0

        # --- NAME FIX LOGIC ---
        raw_name_bytes = unpacked[1]
        fixed_name_parts = []
        for i in range(0, 16, 4):
            chunk = raw_name_bytes[i:i+4]
            fixed_name_parts.append(chunk[::-1])
        hname = b"".join(fixed_name_parts).decode('utf-8').strip('\x00')

        # 3. Determine total Bin Count to read from payload
        if h_type == 3:   # TProfile: 3 values per bin (Mean, RMS, Entries)
            n_bins = n_bins_x * 3 
        elif h_type == 2: # TH2F: 2D grid with under/overflow
            n_bins = (n_bins_x + 2) * (n_bins_y + 2)
        else:             # TH1F: 1D with under/overflow
            n_bins = (n_bins_x + 2)
            
        bin_start = 56
        actual_available = (len(payload) - bin_start) // 4
        n_bins_to_read = min(n_bins, actual_available)
        
        # 4. Extract the bin data
        bin_values = struct.unpack(f'>{n_bins_to_read}I', payload[bin_start:bin_start + (n_bins_to_read * 4)])
        
        print(f"   -> Successfully Decoded {hname}: Run {run_num}, Type {h_type}")
        
        # 5. Insert into Database with all ESSENTIAL metadata
        insert_to_db(
            run_num, hname, bin_values, 
            h_type, n_bins_x, x_min, x_max, n_bins_y, y_min, y_max
        )

    except Exception as e:
        print(f"Decoding Error: {e}")

client = mqtt.Client()
client.on_message = on_message
client.connect(conf['mqtt']['broker'], conf['mqtt']['port'])
client.subscribe(conf['mqtt']['topic'])

print("TOF Bridge is running...")
client.loop_forever()
