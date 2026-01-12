project(GRAMS_TOF_Analysis)

message("${BMagenta}[============ Entering Analysis Subdirectory ============]${ColReset}")

set(GRAMS_TOF_ANALYSIS_FILES
	src/TOF_ChannelConversion.cpp
	src/TOF_TdcQdcCalibration.cpp
	src/TOF_TreeData.cpp
	src/TOF_Attributes.cpp
	src/TOF_Fitting.cpp
	src/TOF_ParameterScan.cpp
	src/TOF_CoincidenceEvents.cpp
	src/TOF_CoincidenceChannelInfo.cpp
	src/TOF_DiscriminatorCalibation.cpp
)

set(GRAMS_TOF_ANALYSIS_HEADERS
  TOF_ChannelConversion.h
  TOF_TdcQdcCalibration.h
  TOF_TreeData.h
  TOF_Constants.h
  TOF_Struct.h
  TOF_Attributes.h
  TOF_Fitting.h
  TOF_ParameterScan.h
  TOF_CoincidenceEvents.h
  TOF_CoincidenceChannelInfo.h
  TOF_DiscriminatorCalibation.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ROOT dictionary
root_generate_dictionary( 
	GramsTofAnalysisDict 
	${GRAMS_TOF_ANALYSIS_HEADERS} 
	LINKDEF include/TOF_Linkdef.h 
	MODULE GramsTofAnalysisLib
)

## the generated dictionary files
set(GRAMS_TOF_DICT
    ${CMAKE_CURRENT_BINARY_DIR}/GramsTofAnalysisDict.cxx
)

## Build a shared lib
## Add the generated dictionary *.cxx files to lib
add_library(GramsTofAnalysisLib SHARED
    ${GRAMS_TOF_ANALYSIS_FILES}
    ${GRAMS_TOF_DICT}
)

target_include_directories(GramsTofAnalysisLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/Analysis>
)
target_link_libraries(GramsTofAnalysisLib PUBLIC ${ROOT_LIBRARIES})

# 빌드 순서 보장: dictionary 먼저 생성
#add_dependencies(GramsTofAnalysisLib GramsTofAnalysisDict)


# 라이브러리 설치
install(TARGETS GramsTofAnalysisLib
        EXPORT GramsTofLibraryTargets ### diff
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

# header files are copied to include
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/Analysis)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/macro/ DESTINATION macro)

add_subdirectory(scripts_c)

