
from petsys import daqd, fe_power
import sys
import time
import argparse

def print_power_telemetry(conn, portID, slaveID, stage_label=""):
    """
    Reads and prints the power status, health rails, and raw telemetry bytes of the given board.
    """
    fem_status = fe_power.get_fem_power_status(conn, portID, slaveID)
    fem_str = "ON (Drawing +0.3A)" if fem_status else "OFF (Low idle current)"

    bias_status = fe_power.get_bias_power_status(conn, portID, slaveID)
    bias_str = "ON" if bias_status else "OFF"

    try:
        fb_reg = conn.read_config_register(portID, slaveID, fe_power.FEM_POWER_FB_REG_LEN, fe_power.FEM_POWER_FB_REG)
        rail_stable = "GOOD" if (fb_reg & 0b1) else "NO POWER / UNDERVOLTAGE"
        
        raw_telemetry_hex = f"0x{fb_reg:016X}"
        
        bit_flags = f"General_PG={fb_reg & 0b1}, Bias_PG={(fb_reg >> 1) & 0b1}"
    except Exception:
        rail_stable = "UNKNOWN (Read Error)"
        raw_telemetry_hex = "N/A"
        bit_flags = "N/A"

    print(f"[{stage_label}] Board (Port {portID}, Slave {slaveID}):")
    print(f"  -> ASIC / FEM Power : {fem_str}")
    print(f"  -> SiPM Bias Power  : {bias_str}")
    print(f"  -> Hardware Rail    : {rail_stable}")
    print(f"  -> Raw Telemetry Reg: {raw_telemetry_hex} ({bit_flags})")
    print("-" * 50)

def toggle_system_power(state="off"):
    if state not in ["on", "off"]:
        print("Invalid state. Use 'on' or 'off'.")
        return False

    # Connect to the DAQ service daemon
    conn = daqd.Connection()
    
    active_boards = conn.getActiveFEBDs()
    if not active_boards:
        print("No active FEB/D boards found.")
        return False

    print(f"\n=== STARTING POWER TOGGLE SYSTEM (Target State: {state.upper()}) ===\n")

    for portID, slaveID in active_boards:
        # STEP 1: Monitor current status before the switch
        print_power_telemetry(conn, portID, slaveID, stage_label="1. BEFORE TOGGLE")
        
        # STEP 2: Toggle the power status on/off
        print(f"Executing: Switching power to {state.upper()}...")
        try:
            fe_power.set_fem_power(conn, portID, slaveID, state)
            # Give the physical DC-DC converters a brief moment to stabilize current draw
            time.sleep(0.1) 
        except fe_power.PowerGoodError:
            print(f"ERROR: Board at Port {portID} failed the Power-Good hardware check!")
            continue
        except Exception as e:
            print(f"ERROR: Failed to communicate with board: {e}")
            continue

        # STEP 3: Monitor current status after the switch to confirm
        print_power_telemetry(conn, portID, slaveID, stage_label="2. AFTER TOGGLE")
    return True

def safe_toggle_system_power(mode="off"):
    try:
        return toggle_system_power(mode)
    except ProcessLookupError:
        print("[Python] Worker process already exited during cleanup. Continuing...")
        return True
    except Exception as e:
        print("[Python] Caught exception:", e)
        traceback.print_exc()
        return False

def main(argv):
    parser = argparse.ArgumentParser(description='Control TOFPET2 ASIC/FEM Power States')
    
    parser.add_argument("--mode", type=str, choices=["on", "off"], default="off")
    args = parser.parse_args(argv[1:])

    safe_toggle_system_power(args.mode)

if __name__ == '__main__' and not hasattr(sys, '_called_from_c'):
    main(sys.argv)
