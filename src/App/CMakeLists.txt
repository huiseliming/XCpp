set(CURRENT_MODULE_NAME "XApp")

file(GLOB_RECURSE ${CURRENT_MODULE_NAME}_HEADERS *.h *.hh *.hpp *.hxx)
file(GLOB_RECURSE ${CURRENT_MODULE_NAME}_SOURCES *.c *.cc *.cpp *.cxx)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX Core
  FILES ${${CURRENT_MODULE_NAME}_HEADERS} ${${CURRENT_MODULE_NAME}_SOURCES}
)
# library target 
add_library(${CURRENT_MODULE_NAME} SHARED ${${CURRENT_MODULE_NAME}_HEADERS} ${${CURRENT_MODULE_NAME}_SOURCES})
# include directories
target_include_directories(${CURRENT_MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
# link libraries
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC XCore XRenderer XImGui)
# export
include(GenerateExportHeader)
string(TOUPPER ${CURRENT_MODULE_NAME} CK_MODULE_NAME_UPPER)
generate_export_header(${CURRENT_MODULE_NAME}
    EXPORT_MACRO_NAME ${CK_MODULE_NAME_UPPER}_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME}Export.h
)
# precompile header
target_precompile_headers(${CURRENT_MODULE_NAME}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME}Export.h
)
