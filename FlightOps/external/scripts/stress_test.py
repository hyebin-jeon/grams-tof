from petsys import daqd, config
import os
import time

# 1. Setup Paths
glib_dir = os.environ.get('GLIB', '/home/ksakai/work/source/grams-tof/00build/00install')
config_path = os.path.join(glib_dir, 'config', 'config.ini')
output_prefix = "./stress_test_data"

# 2. Connect and Initialize
daq_conn = daqd.Connection()
daq_conn.initializeSystem()

# Masking out QDCMODE_MAP to avoid the missing .tsv error
mask = config.LOAD_ALL ^ config.LOAD_QDCMODE_MAP
systemConfig = config.ConfigFromFile(config_path, loadMask=mask)

# 3. Load to Hardware (Initial State)
print("Loading base configuration...")
systemConfig.loadToHardware(daq_conn, bias_enable=config.APPLY_BIAS_OFF, qdc_mode="qdc")

# 4. Open Acquisition
print(f"Opening data stream at: {output_prefix}.rawf")
#daq_conn.openRawAcquisition(output_prefix, calMode=True)
daq_conn.openRawAcquisition(output_prefix, calMode=False)

# 5. Apply Calibration-Specific Settings
asicsConfig = daq_conn.getAsicsConfig()

print("Configuring ASICs using Calibration-style settings...")
for asic_key, ac in asicsConfig.items():
    # Set the global mirror bias found in acquire_qdc_calibration.py
    ac.globalConfig.setValue("imirror_bias_top", 23)
    
    for cc in ac.channelConfig:
        # Disable all external triggers
        cc.setValue("trigger_mode_2_t", 0)
        cc.setValue("trigger_mode_2_e", 0)
        cc.setValue("trigger_mode_2_q", 0)
        cc.setValue("trigger_mode_2_b", 0)
        
        # Set QDC gate windows
        cc.setValue("min_intg_time", 0)
        cc.setValue("max_intg_time", 127)
        
        # FORCE a low threshold so the pulse definitely triggers
        # DAC 30 is usually a very sensitive setting for calibration
        cc.setValue("vth_t1", 30) 
        
        # Start everyone as Disabled (0b11)
        cc.setValue("trigger_mode_1", 0b11)

# Enable Pulse for Every 4th Channel
print("Enabling every 4th channel for stress...")
for ac in list(asicsConfig.values()):
    for i in range(0, 64, 4):
        ac.channelConfig[i].setValue("trigger_mode_1", 0b01)

daq_conn.setAsicsConfig(asicsConfig)

# 6. Setup the Pulse (1 kHz)
# Using a longer pulse (100) as seen in calibration
print("Starting 1 kHz internal pulse (Length 100)...")
daq_conn.set_test_pulse_febds(100, 20000, 0.0, False)
time.sleep(0.2) # Small pause for hardware to settle

# 7. Start Acquisition
print("Starting Stress Test (60s)...")
# Note: In acquire(), the 2nd and 3rd args are just tags for the file header
daq_conn.acquire(60, 100, 0.0)

# 8. Cleanup
daq_conn.closeAcquisition()
print("\nSuccess! Check file size now.")
