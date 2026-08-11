import argparse, sys, time
import pandas as pd
from petsys import daqd, fe_temperature, config, fe_power # type: ignore
from math import fabs
from collections import defaultdict
import traceback
import os

def write_table_header(fd, sensor_list):
    locations_list = [sensor.get_location() for sensor in sensor_list]
    fd.write("# Absolute Sensor ID format : portID_slaveID_moduleID_SensorID\n")
    fd.write("# Sensor ID is:\n")
    if any(location[3] > 1 for location in locations_list): #module is FEM256
        fd.write("# A0 - Sensor on FEM 256 (ASIC 0)\n")
        fd.write("# A1 - Sensor on FEM 256 (ASIC 1)\n")
        fd.write("# A2 - Sensor on FEM 256 (ASIC 2)\n")
        fd.write("# A3 - Sensor on FEM 256 (ASIC 3)\n")  
        fd.write("# S0 - Sensor on FEB/S (SIPM read by ASIC 0)\n")
        fd.write("# S1 - Sensor on FEB/S (SIPM read by ASIC 1)\n")
        fd.write("# S2 - Sensor on FEB/S (SIPM read by ASIC 2)\n")
        fd.write("# S3 - Sensor on FEB/S (SIPM read by ASIC 3)\n")                       
    else:
        fd.write("# A0 - Sensor on FEB/A (ASIC 0) connected to port J1 of the FEB/I\n")
        fd.write("# A1 - Sensor on FEB/A (ASIC 1) connected to port J2 of the FEB/I\n")
        fd.write("# S0 - Sensor on FEB/S (SIPM) connected to port J1 of the FEB/I\n")
        fd.write("# S1 - Sensor on FEB/S (SIPM) connected to port J2 of the FEB/I\n")               

    fd.write("#\n#DAQ timestamp\tSystem time")

    for (portID,slaveID, moduleID, sensorID, sensorPlace) in locations_list:
         sid = 'A'+ str(sensorID)   
         if sensorPlace == "sipm":
             sid = 'S'+ str(sensorID)   
         fd.write("\t%d_%d_%d_%s" % (portID,slaveID,moduleID,sid))
    fd.write("\n")
         
def write_table_row(fd, sensor_list, timestamp):
    T = []
    fd.write("%d\t%d" %(timestamp, time.time()))
    for sensor in sensor_list:
        temp = sensor.get_temperature()
        fd.write("\t%.2f" % temp)
        if sensor.get_location()[4] == "asic":
            T.append(temp)
    fd.write("\n")
    fd.flush()
    return T

def tableize(df): # Thanks https://stackoverflow.com/a/62046440
    if not isinstance(df, pd.DataFrame):
        return
    df_columns = df.columns.tolist() 
    max_len_in_lst = lambda lst: len(sorted(lst, reverse=True, key=len)[0])
    align_center = lambda st, sz: "{0}{1}{0}".format(" "*(1+(sz-len(st))//2), st)[:sz] if len(st) < sz else st
    align_right = lambda st, sz: "{0}{1} ".format(" "*(sz-len(st)-1), st) if len(st) < sz else st
    max_col_len = max_len_in_lst(df_columns)
    max_val_len_for_col = dict([(col, max_len_in_lst(df.iloc[:,idx].astype('str'))) for idx, col in enumerate(df_columns)])
    col_sizes = dict([(col, 1 + max(max_val_len_for_col.get(col, 0), max_col_len)) for col in df_columns])
    build_hline = lambda row: '+'.join(['-' * col_sizes[col] for col in row]).join(['+', '+'])
    build_data = lambda row, align: "|".join([align(str(val), col_sizes[df_columns[idx]]) for idx, val in enumerate(row)]).join(['|', '|'])
    hline = build_hline(df_columns)
    out = [hline, build_data(df_columns, align_center), hline]
    for _, row in df.iterrows():
        out.append(build_data(row.tolist(), align_right))
    out.append(hline)
    print( "\n".join(out))

def print_table(sensor_list, show_sipm=True):
    # Find out max number of ASICs per FEM and build sensor column list 
    n_asics_max = max([sensor.get_location() for sensor in sensor_list], key=lambda x:x[3])[3] + 1

    types_lst = ['asic']
    if show_sipm: types_lst.append('sipm')

    for type in types_lst:
        print('|')
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))
        print('+','-' * 1,'> ',f'{type.upper()} sensors @ ',f'{timestamp}', sep = '')
        sensor_cols =  [f'{type[0].upper()}{idx} Sensor' for idx in range(n_asics_max)]
        # Create DF
        df_cols = ['portID', 'slaveID', 'moduleID'] + sensor_cols
        df = pd.DataFrame(columns=df_cols)
        # Group by module
        module_lst = defaultdict(list)
        for (p, s, m, l, t), temp in [ (sensor.get_location(), sensor.get_temperature()) for sensor in sensor_list]:
            module_lst[(p,s,m)].append((l , t, temp))
        # Fill DF rows
        for idx in module_lst:
            entry = {'portID': idx[0], 'slaveID': idx[1], 'moduleID': idx[2]}
            for sensor in module_lst[idx]:
                if sensor[1] == type:
                    new_temp = f'{sensor[2]:.2f} ºC'
                    new_col  = f'{type[0].upper()}{sensor[0]} Sensor' 
                    entry[new_col] = new_temp
            df.loc[len(df)] = entry
        # Print table with timestamp
        tableize(df.fillna("-"))
    print('')


# --- FUNCTION 1: Temperature Stabilization ---
def read_temperature_sensors(acq_time=600.0, interval=60.0, fileName="/dev/null", startup=False, debug=False, quiet=False):
    """Monitors ASIC temperature stabilization (600s max timeout, 60s check interval, 0.2 C delta quit threshold)."""
    if not quiet:
        print("read_temperature_sensors (Stabilization Mode) called with:", acq_time, interval, fileName, startup, debug)

    conn = daqd.Connection()

    for portID, slaveID in conn.getActiveFEBDs():
        if not fe_power.get_fem_power_status(conn, portID, slaveID):
            if not quiet:
                print(f'WARNING: FEM Power for (portID, slaveID) = ({portID}, {slaveID}) is OFF.')
            fe_power.set_fem_power(conn, portID, slaveID, "on")
            time.sleep(0.01)

    sensor_list = fe_temperature.get_sensor_list(conn, debug=debug)
    if not sensor_list:
        if not quiet:
            print("ERROR: No sensors found. Check connections and power.")
        return False
    else:
        sensor_list.sort(key=lambda x: x.get_location())

    file_exists_and_not_empty = os.path.exists(fileName) and os.path.getsize(fileName) > 0
    dataFile = open(fileName, "a" if file_exists_and_not_empty else "w")

    if not file_exists_and_not_empty:
        write_table_header(dataFile, sensor_list)

    temp_cache = write_table_row(dataFile, sensor_list, conn.getCurrentTimeTag())

    if not quiet:
        print_table(sensor_list, show_sipm=False)

    STARTUP_TIME = acq_time if acq_time > 0 else 600.0
    STARTUP_INTERVAL = interval if interval > 0 else 60.0
    STARTUP_T_DELTA = 0.2

    tNow = time.time()
    tEnd = tNow + STARTUP_TIME
    stable = False

    while not stable:
        if not quiet:
            print(f'INFO: Stabilizing ASIC temperature. Waiting for {STARTUP_INTERVAL} seconds...\n')
        time.sleep(STARTUP_INTERVAL)
        tNow += STARTUP_INTERVAL
        
        temp_now = write_table_row(dataFile, sensor_list, conn.getCurrentTimeTag())
        if not quiet:
            print_table(sensor_list, show_sipm=False)

        stable = True
        for t1, t2 in zip(temp_now, temp_cache):
            if fabs(t1 - t2) > STARTUP_T_DELTA:
                stable = False
                break
        temp_cache = temp_now.copy()

        if stable:
            if not quiet:
                print(f'INFO: ASIC temperatures stable (diff <= {STARTUP_T_DELTA} C) over last {STARTUP_INTERVAL} s. Quitting stabilization.')
            break
        elif tNow >= tEnd:
            if not quiet:
                print(f'WARNING: Reached timeout limit of {STARTUP_TIME} seconds before stabilization.')
            break

    dataFile.close()
    if not quiet:
        print("[read_temperature_sensors] Stabilization phase finished.")
    return True


# --- FUNCTION 2: Single-Shot Data Dumping ---
def startTempRecord(fileName="/dev/null", debug=False, quiet=False):
    """Single-shot sensor measurement: records one temperature snapshot to file and returns."""
    if not quiet:
        print("startTempRecord (Single-Shot Mode) called with:", fileName, debug)

    conn = daqd.Connection()

    for portID, slaveID in conn.getActiveFEBDs():
        if not fe_power.get_fem_power_status(conn, portID, slaveID):
            if not quiet:
                print(f'WARNING: FEM Power for (portID, slaveID) = ({portID}, {slaveID}) is OFF.')
            fe_power.set_fem_power(conn, portID, slaveID, "on")
            time.sleep(0.01)

    sensor_list = fe_temperature.get_sensor_list(conn, debug=debug)
    if not sensor_list:
        if not quiet:
            print("ERROR: No sensors found. Check connections and power.")
        return False
    else:
        sensor_list.sort(key=lambda x: x.get_location())

    file_exists_and_not_empty = os.path.exists(fileName) and os.path.getsize(fileName) > 0
    dataFile = open(fileName, "a" if file_exists_and_not_empty else "w")

    if not file_exists_and_not_empty:
        write_table_header(dataFile, sensor_list)

    write_table_row(dataFile, sensor_list, conn.getCurrentTimeTag())
    if not quiet:
        print_table(sensor_list)

    dataFile.close()
    if not quiet:
        print("[startTempRecord] Single-shot measurement recorded.")
    return True


def safe_read_temperature_sensors(acq_time=600.0, interval=60.0, fileName="/dev/null", startup=False, debug=False, quiet=False, record_mode=False):
    try:
        if record_mode:
            return startTempRecord(fileName, debug, quiet)
        else:
            return read_temperature_sensors(acq_time, interval, fileName, startup, debug, quiet)
    except Exception as e:
        if not quiet:
            print("[Python] Caught exception:", e)
            if os.environ.get("DEBUG"):
                traceback.print_exc()
        return False


def main(argv):
    parser = argparse.ArgumentParser(description='Measure temperature from all connected sensors')
    parser.add_argument("--time", type=float, default=600.0, help="Acquisition/timeout time (in seconds)")
    parser.add_argument("--interval", type=float, default=60.0, help="Measurement interval (in seconds)")
    parser.add_argument("-o", type=str, default="/dev/null", dest="fileName", help="Data filename")
    parser.add_argument("--startup", action="store_true", help="Check temperature stability when starting up")
    parser.add_argument("--record", action="store_true", help="Run single-shot temperature record mode")
    parser.add_argument("--debug", action="store_true", help="Enable debug mode")
    parser.add_argument("--quiet", action="store_true", help="Suppress terminal output and tables")

    args = parser.parse_args()

    ok = safe_read_temperature_sensors(args.time, args.interval, args.fileName, args.startup, args.debug, args.quiet, record_mode=args.record)
    return 0 if ok else 1


if __name__ == '__main__' and not hasattr(sys, '_called_from_c'):
    sys.exit(main(sys.argv))
