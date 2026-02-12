#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from petsys import daqd, config, fe_temperature, fe_power
from copy import deepcopy
import argparse
from time import sleep, time
from sys import stdout
import traceback
import os
import sys

def acquire_active_asic_list(config_file, out_file_prefix):
    print("acquire_threshold_calibration:", config_file, out_file_prefix, noise_reads, dark_reads, ext_bias, "mode:", mode)

    systemConfig = config.ConfigFromFile(config_file, loadMask=config.LOAD_BIAS_CALIBRATION | config.LOAD_BIAS_SETTINGS)
    conn = daqd.Connection()

    # Ensure FEM power is ON
    for portID, slaveID in conn.getActiveFEBDs():
        if not fe_power.get_fem_power_status(conn, portID, slaveID):
            print(f'WARNING: FEM Power for (portID, slaveID) = ({portID}, {slaveID}) is OFF.')
            fe_power.set_fem_power(conn, portID, slaveID, "on")
            time.sleep(0.01)

    # Check sensors
    sensor_list = fe_temperature.get_sensor_list(conn)
    if not sensor_list:
        print("ERROR: No sensors found. Check connections and power.")
        return False

    conn.initializeSystem()
    asicsConfig = conn.getAsicsConfig()
    activeAsics = conn.getActiveAsics()

	  # Ouput file
    outputFile = open(out_file_prefix + ".tsv", "w")
    outputFile.write("#portID\tslaveID\tchipID\n")
    for (portID, slaveID, chipID) in activeAsics:
        outputFile.write("%d\t%d\t%d\n" %
                         (portID, slaveID, chipD))

    print("[acquire_active_asic_list] Done.")
    return True

def safe_acquire_active_asic_list(config_file, out_file_prefix):
    try:
        return acquire_active_asic_list(config_file, out_file_prefix)
    except ProcessLookupError:
        print("[Python] Worker process already exited during cleanup. Continuing...")
        return True
    except Exception as e:
        print("[Python] Caught exception:", e)
        traceback.print_exc()
        return False

def main(argv):
    parser = argparse.ArgumentParser(description='Acquire data for threshold calibration')
    parser.add_argument("--config", type=str, required=True)
    parser.add_argument("-o", type=str, dest="outFilePrefix", required=True)
    #args = parser.parse_args(argv[1:])
    args = parser.parse_args()

    safe_acquire_active_asic_list(args.config, args.outFilePrefix)
    
if __name__ == '__main__' and not hasattr(sys, '_called_from_c'):
    main(sys.argv)
