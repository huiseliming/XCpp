set(CURRENT_MODULE_NAME "XCore")

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
# boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC ${Boost_LIBRARIES})
# fmt
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC fmt::fmt)
# spdlog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC spdlog::spdlog_header_only)
# cxxopts
find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC cxxopts::cxxopts)
# sdl2
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${CURRENT_MODULE_NAME}
    PUBLIC $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    PUBLIC $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
# glm
find_package(glm CONFIG REQUIRED)
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC glm::glm)
# compile time info
target_compile_definitions(${CURRENT_MODULE_NAME}
    PRIVATE X_VERSION="v0.0.0"
    PRIVATE $<$<CONFIG:Debug>:         X_DEBUG=1 X_BUILD_TYPE="Debug">
    PRIVATE $<$<CONFIG:Release>:       X_DEBUG=0 X_BUILD_TYPE="Release">
    PRIVATE $<$<CONFIG:MinSizeRel>:    X_DEBUG=0 X_BUILD_TYPE="MinSizeRel">
    PRIVATE $<$<CONFIG:RelWithDebInfo>:X_DEBUG=0 X_BUILD_TYPE="RelWithDebInfo">
)
# export
include(GenerateExportHeader)
string(TOUPPER ${CURRENT_MODULE_NAME} CURRENT_MODULE_NAME_UPPER)
generate_export_header(${CURRENT_MODULE_NAME}
    EXPORT_MACRO_NAME ${CURRENT_MODULE_NAME_UPPER}_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME}Export.h
)
# precompile header
target_precompile_headers(${CURRENT_MODULE_NAME}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME}Export.h
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/CoreFwd.h
)