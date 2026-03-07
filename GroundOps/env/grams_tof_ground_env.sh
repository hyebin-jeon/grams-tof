#!/bin/bash

# --- MQTT Configuration ---
#export PGRAMS_MOSQUITTO_HOST=10.150.111.190 #from Shota
export PGRAMS_MOSQUITTO_HOST="localhost"
export PGRAMS_MOSQUITTO_PORT=1883
export PGRAMS_MOSQUITTO_TOPIC="TOF_ground_telemetry"
export PGRAMS_MOSQUITTO_USER=""
export PGRAMS_MOSQUITTO_PASSWD=""

# --- MySQL Configuration ---
export TOF_MYSQL_HOST="127.0.0.1"
export TOF_MYSQL_USER="pgrams_user"
export TOF_MYSQL_DB="grams_tof"
export TOF_MYSQL_PASS="pgrams_user"

# --- Project Paths ---
export GLIB=$HOME/work/source/grams-tof/00build/00install
export LD_LIBRARY_PATH=$GLIB/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$GLIB:$PYTHONPATH
export TOF_BRIDGE_SCRIPT="$GLIB/ground/bin/tof_bridge.py"

#export TOFDATA=$HOME/work/grams/proc/tofdata
export TOFDATA=$HOME/work/grams/proc/tof_data

export TOF_ROOT_DIR="$HOME/work/source/grams-tof"

echo "GRAMS-TOF Ground Environment Loaded"
echo "   MQTT Host: $PGRAMS_MOSQUITTO_HOST"
echo "   MySQL DB:  $TOF_MYSQL_DB"
