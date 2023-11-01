set(ANGEL_SCRIPT_VER 2.36.1)
set(ANGEL_SCRIPT_ZIP angelscript_${ANGEL_SCRIPT_VER}.zip)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/angelscript_${ANGEL_SCRIPT_VER}.zip")
    file(DOWNLOAD
        "https://www.angelcode.com/angelscript/sdk/files/angelscript_${ANGEL_SCRIPT_VER}.zip"
        ${CMAKE_CURRENT_SOURCE_DIR}/angelscript_${ANGEL_SCRIPT_VER}.zip
        SHOW_PROGRESS
    )
    execute_process(COMMAND
        ${CMAKE_COMMAND} -E tar -xzf ${CMAKE_CURRENT_SOURCE_DIR}/angelscript_${ANGEL_SCRIPT_VER}.zip
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sdk/angelscript/projects/cmake)

file(GLOB_RECURSE XAngelScript_HEADERS sdk/add_on/*.h)
list(FILTER XAngelScript_HEADERS EXCLUDE REGEX ".*sdk/add_on/autowrapper/aswrappedcall.h$")
file(GLOB_RECURSE XAngelScript_SOURCES sdk/add_on/*.cpp)
list(FILTER XAngelScript_SOURCES EXCLUDE REGEX ".*sdk/add_on/autowrapper/generator/generateheader.cpp$")

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX src
  FILES ${XAngelScript_HEADERS} ${XAngelScript_SOURCES}
)

add_library(XAngelScript STATIC 
    ${XAngelScript_HEADERS} 
    ${XAngelScript_SOURCES}
)

target_include_directories(XAngelScript PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/)
target_link_libraries(XAngelScript PUBLIC ${ANGELSCRIPT_LIBRARY_NAME})

