# -*- coding: utf-8 -*-
import time
import threading
import sys
from datetime import datetime  # FIXED: Required for timestamping

# --- Configuration ---
BIAS_LOG_FILE = "bias_stream.log" 

# Set USE_MOCK_BIAS to False when connected to physical hardware
#USE_MOCK_BIAS = True       
USE_MOCK_BIAS = False 
BIAS_PORT = "/dev/ttyUSB0" 


# --- SiPM Bias Controller ---
class SiPMBiasBoardController:
    """Controller for the SiPM Bias Board with continuous stream logging."""
    def __init__(self, serial_conn, log_file=BIAS_LOG_FILE):
        self.ser = serial_conn
        self.log_file = log_file
        self.running = True
        self.lock = threading.Lock()
        
        self.current_state = "off"

        # Start continuous background reader thread to dump telemetry to file
        self._reader_thread = threading.Thread(target=self._continuous_logger, daemon=True)
        self._reader_thread.start()

    def _continuous_logger(self):
        """Continuously reads incoming telemetry/responses and dumps to log file."""
        while self.running:
            try:
                with self.lock:
                    line = self.ser.readline().decode("ascii", errors="replace")
                
                if line:
                    clean_line = line.rstrip("\r\n")
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
                    
                    formatted_line = f"[{timestamp}] {clean_line}\n"
                    
                    with open(self.log_file, "a", encoding="utf-8") as f:
                        f.write(formatted_line)
                        f.flush()
                else:
                    time.sleep(0.05)
                    
            except Exception as e:
                time.sleep(0.1)

    def send_command(self, cmd: str) -> str:
        """Sends command string to the bias board safely."""
        print(f"    [TX] -> {cmd}")
        full_cmd = f"{cmd}\r\n".encode("ascii")
        
        with self.lock:
            self.ser.write(full_cmd)
            if hasattr(self.ser, "flush"):
                self.ser.flush()  # Ensure bytes are immediately pushed to hardware
            
        return f"Command '{cmd}' sent"

    def set_all_channels_state(self, state: str):
        """High-level helper for state transitions."""
        state = state.lower()
        if state not in ("off", "prebreakdown", "bias_on"):
            print("Invalid state. Use 'off', 'prebreakdown', or 'bias_on'.")
            return

        print(f">>> Transitioning state: '{self.current_state}' -> '{state}'...")
        active_channels = [0, 1, 2, 4, 6, 7]

        # 1. Handle Channel Enable/Disable ONLY during Off <-> Active transitions
        if self.current_state == "off" and state in ("prebreakdown", "bias_on"):
            for ch in active_channels:
                self.send_command(f"enable {ch} on")
        elif self.current_state in ("prebreakdown", "bias_on") and state == "off":
            for ch in active_channels:
                self.send_command(f"enable {ch} off")

        # 2. Adjust offset voltage for active states
        if state == "prebreakdown":
            self.send_command("voffset 16.0")
        elif state == "bias_on":
            self.send_command("voffset 0.0")

        self.current_state = state

    def set_global_bias_state(self, state: str):
        self.set_all_channels_state(state)
        time.sleep(2)
        self.send_command("p")

    def stop(self):
        """Clean shutdown helper."""
        self.running = False
        if self.ser and hasattr(self.ser, "close"):
            try:
                self.ser.close()
            except Exception:
                pass


def get_bias_controller():
    """Factory helper using conditional imports for Mock vs Hardware."""
    if USE_MOCK_BIAS:
        from mock_bias_board import MockSerial
        print(">>> Operating in MOCK mode.")
        serial_conn = MockSerial()
    else:
        import serial
        print(f">>> Connecting to hardware on {BIAS_PORT}...")
        serial_conn = serial.Serial(BIAS_PORT, baudrate=115200, timeout=0.2)
    
    return SiPMBiasBoardController(serial_conn)


def initialize_board(bias_ctrl):
    """Sends baseline default commands upon startup."""
    print("--- Initializing Bias Controller ---")
    bias_ctrl.send_command("data on")
    bias_ctrl.send_command("echo on")
    bias_ctrl.send_command("mode auto")
    bias_ctrl.send_command("vdef 64 163")
    bias_ctrl.send_command("vdef 65 163")
    bias_ctrl.send_command("vdef 72 164")
    bias_ctrl.send_command("vdef 73 164")
    bias_ctrl.send_command("p")


def interactive_cli():
    bias_ctrl = get_bias_controller()
    initialize_board(bias_ctrl)

    print("\n" + "=" * 60)
    print("MANUAL BIAS BOARD CONTROL INTERFACE")
    print("Background packet logging is RUNNING to file: " + BIAS_LOG_FILE)
    print("-" * 60)
    print("Quick Macro Commands:")
    print("  state off          - Transition board to OFF state")
    print("  state prebreakdown - Transition board to PREBREAKDOWN state")
    print("  state bias_on      - Transition board to BIAS_ON state")
    print("  exit / quit        - Exit application")
    print("  Or type any RAW command to send directly to the board (e.g., 'p', 'voffset 16.0')")
    print("=" * 60 + "\n")

    try:
        while True:
            user_cmd = input("bias> ").strip()
            
            if not user_cmd:
                continue
                
            if user_cmd.lower() in ("exit", "quit"):
                print("Shutting down controller...")
                bias_ctrl.stop()
                break

            if user_cmd.lower().startswith("state "):
                target_state = user_cmd.split(" ", 1)[1].strip()
                bias_ctrl.set_global_bias_state(target_state)
            else:
                bias_ctrl.send_command(user_cmd)

    except KeyboardInterrupt:
        print("\nExiting program...")
        bias_ctrl.stop()

if __name__ == "__main__":
    interactive_cli()
