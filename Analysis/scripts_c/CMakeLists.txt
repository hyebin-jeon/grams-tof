project(Grams_Tof_Analysis_Scripts_C)
message("${BMagenta}[============ Entering TOF Scripts C Subdirectory ============]${ColReset}")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

find_package(Boost REQUIRED COMPONENTS random)

# Make a lib for all the stuff
add_library(GramsTofAnalysisScriptsCLib SHARED ${SOURCES})
target_include_directories(GramsTofAnalysisScriptsCLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/scripts_c>
)
target_link_libraries(GramsTofAnalysisScriptsCLib PUBLIC ${ROOT_LIBRARIES})
target_link_libraries(GramsTofAnalysisScriptsCLib PUBLIC GramsTofAnalysisLib)

add_executable(process_tofCoinEvtQA tools/process_tofCoinEvtQA.cpp)
target_link_libraries(process_tofCoinEvtQA PRIVATE GramsTofAnalysisScriptsCLib)

install(TARGETS GramsTofAnalysisScriptsCLib
    EXPORT GramsTofLibraryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/scripts_c)

install(TARGETS process_tofCoinEvtQA
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

