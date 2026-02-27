cmake_minimum_required(VERSION 3.20)

set(GROUND_INSTALL_DIR "ground")

# 1. Install the bridge script into ground/bin
install(PROGRAMS 
    scripts/tof_bridge.py
    scripts/setup_db.py 
    DESTINATION ${GROUND_INSTALL_DIR}/bin
)

# 2. Install the configuration into ground/config
install(FILES 
    config/bridge_config.json 
    DESTINATION ${GROUND_INSTALL_DIR}/config
)

# 3. Install the SQL schema into ground/sql
install(FILES 
    sql/create_tof_monitor.sql 
    DESTINATION ${GROUND_INSTALL_DIR}/sql
)

# 4. Modified: Install the environment setup script into ground/env
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/env/grams_tof_ground_env.sh")
    install(FILES 
        env/grams_tof_ground_env.sh 
        DESTINATION ${GROUND_INSTALL_DIR}/env
    )
endif()

message(STATUS "GroundOps configured: Files will be installed to ${CMAKE_INSTALL_PREFIX}/${GROUND_INSTALL_DIR}")
