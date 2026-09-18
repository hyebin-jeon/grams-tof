project(Grams_Tof_FlightOps_Daemon)
message("${BMagenta}[============ Entering TOF FlightOps Daemon Subdirectory ============]${ColReset}")

add_subdirectory(quill)

# ASIO Search
file(GLOB ASIO_SEARCH_CUSTOM_PATH "$ENV{HOME}/asio*/include")
find_path(ASIO_INCLUDE_DIR asio.hpp
    HINTS ${ASIO_SEARCH_CUSTOM_PATH} /usr/include /usr/local/include
)

# Communication Code definitions
if(DEFINED ENV{BALLOON_BASE_DIR})
    set(COMM_CODES_INCLUDE "$ENV{BALLOON_BASE_DIR}/include")
elseif(DEFINED ENV{TPC_DAQ_BASEDIR})
    set(COMM_CODES_INCLUDE "$ENV{TPC_DAQ_BASEDIR}/PGramsCommCodec/comm_codes")
else()
    set(COMM_CODES_INCLUDE "")
endif()

set(DUMMY_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include_symlink")
file(MAKE_DIRECTORY "${DUMMY_INCLUDE_DIR}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
            "${CMAKE_CURRENT_SOURCE_DIR}/../include" 
            "${DUMMY_INCLUDE_DIR}/FlightOps"
)

# Executables
add_executable(tof_daq tof_daq.cpp)

target_include_directories(tof_daq PRIVATE
    ${DUMMY_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/quill/include
    ${ASIO_INCLUDE_DIR}
    ${COMM_CODES_INCLUDE}
)

target_compile_definitions(tof_daq PUBLIC
    -DQUILL_COMPILE_ACTIVE_LOG_LEVEL=QUILL_COMPILE_ACTIVE_LOG_LEVEL_INFO
)

target_link_libraries(tof_daq PRIVATE
    GramsTofFlightOpsLib
    quill::quill
    ${Boost_LIBRARIES}
    pthread
)

# ----------------- Install Targets -----------------
install(TARGETS tof_daq
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
install(FILES tof_daq.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/service
)
