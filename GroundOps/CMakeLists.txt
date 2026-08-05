cmake_minimum_required(VERSION 3.20)

set(GROUND_INSTALL_DIR "ground")

# 1. Install the bridge script into ground/bin
install(PROGRAMS 
    scripts/tof_bridge.py
    scripts/setup_db.py 
    DESTINATION ${GROUND_INSTALL_DIR}/bin
)

# 2. Install the SQL schema into ground/sql
install(FILES 
    sql/create_tof_monitor.sql 
    DESTINATION ${GROUND_INSTALL_DIR}/sql
)

# 3. Install the environment setup script into ground/env
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/env/grams_tof_ground_env.sh")
    install(FILES 
        env/grams_tof_ground_env.sh 
        DESTINATION ${GROUND_INSTALL_DIR}/env
    )
endif()

# 4. Install Grafana Dashboard JSON into ground/grafana
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/grafana")
    install(DIRECTORY grafana/
        DESTINATION ${GROUND_INSTALL_DIR}/grafana
        FILES_MATCHING PATTERN "*.json"
    )
endif()

# 5. Install proc.log
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/log/setting.log")
    install(FILES 
        log/setting.log 
        DESTINATION ${GROUND_INSTALL_DIR}/log
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
    )
endif()

# 6. Install systemd service unit
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/systemd/tof-bridge.service")
    install(FILES
        systemd/tof-bridge.service
        DESTINATION ${GROUND_INSTALL_DIR}/systemd
    )
endif()

message(STATUS "GroundOps configured: Files will be installed to ${CMAKE_INSTALL_PREFIX}/${GROUND_INSTALL_DIR}")
