project(Grams_Tof_Analysis_Scripts_C)
message("${BMagenta}[============ Entering TOF Scripts C Subdirectory ============]${ColReset}")

## c++17 to use std::filesystem
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/process_convertStg1ToStg2_impl.cpp
    src/process_tofCoinEvtQA_impl.cpp
    src/groundTest_pulse_sync_impl.cpp
)

add_library(GramsTofAnalysisScriptsCLib SHARED ${SOURCES})

target_include_directories(GramsTofAnalysisScriptsCLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/scripts_c>
)

target_link_libraries(GramsTofAnalysisScriptsCLib PUBLIC ${ROOT_LIBRARIES} GramsTofAnalysisLib)

# tofCoinEvtQA
add_executable(process_tofCoinEvtQA tools/process_tofCoinEvtQA.cpp)
target_link_libraries(process_tofCoinEvtQA PRIVATE GramsTofAnalysisScriptsCLib)

# convertStg1ToStg2
add_executable(process_convertStg1ToStg2 tools/process_convertStg1ToStg2.cpp)
target_link_libraries(process_convertStg1ToStg2 PRIVATE GramsTofAnalysisScriptsCLib)

# groundTest_pulse_sync
add_executable( groundTest_pulse_sync tools/groundTest_pulse_sync.cpp)
target_link_libraries(groundTest_pulse_sync PRIVATE GramsTofAnalysisScriptsCLib)

install(TARGETS GramsTofAnalysisScriptsCLib
                process_tofCoinEvtQA
                process_convertStg1ToStg2
        EXPORT GramsTofLibraryTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
        DESTINATION include/scripts_c
)
