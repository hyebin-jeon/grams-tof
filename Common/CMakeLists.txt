project(Grams_Tof_Common)
message("${BMagenta}[============ Entering TOF Common Subdirectory ============]${ColReset}")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

add_subdirectory(external/inih)

# ----------------- fmt Library -----------------
set(FMT_INSTALL ON CACHE BOOL "" FORCE)
set(FMT_STATIC ON CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(external/fmt-7.1.3)
set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(GramsTofLoggerInclude INTERFACE)
target_include_directories(GramsTofLoggerInclude INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/Common>
)

# ----------------- Build Main Shared Library -----------------
add_library(GramsTofCommonLib SHARED ${SOURCES})

target_include_directories(GramsTofCommonLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/Common>
)

target_link_libraries(GramsTofCommonLib PUBLIC
  GramsTofInihLib
  fmt::fmt
)

install(TARGETS GramsTofCommonLib
    EXPORT GramsTofLibraryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Common)
