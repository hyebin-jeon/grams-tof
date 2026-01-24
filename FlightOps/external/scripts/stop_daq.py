#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from petsys import daqd, config
from copy import deepcopy
import argparse
import math
import time
import os
import traceback

def stop_daq():
    print("Stopping DAQ acquisition gracefully...")
    connection = daqd.Connection()
    connection.stopAcquisition() 

    connection.close() 
    import time
    time.sleep(0.1)

    print("DAQ Stopped.")
    return True

def safe_stop_daq():
    try:
        return stop_daq()
    except (ConnectionResetError, BrokenPipeError):
        print("Note: Connection closed by server during shutdown (Safe to ignore).")
        return True

    except Exception as e:
        print("[Python] Caught:", e)
        if os.environ.get("DEBUG"):
            traceback.print_exc()
        return False

def main():
    parser = argparse.ArgumentParser(description="Stopping DAQ acquisition")
    args = parser.parse_args()
    if safe_stop_daq():
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__' and not hasattr(sys, '_called_from_c'):
    main()
