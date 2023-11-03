set(IMGUI_DOCKING_VER 1.89.8)
set(IMGUI_DOCKING_ZIP imgui-${IMGUI_DOCKING_VER}-docking.zip)
set(IMGUI_DOCKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui-${IMGUI_DOCKING_VER}-docking)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/imgui-${IMGUI_DOCKING_VER}-docking.zip")
    file(DOWNLOAD
        "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_DOCKING_VER}-docking.zip"
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui-${IMGUI_DOCKING_VER}-docking.zip
        SHOW_PROGRESS
    )
    execute_process(COMMAND
        ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/imgui-${IMGUI_DOCKING_VER}-docking.zip
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

set(CURRENT_MODULE_NAME "XImGui")
source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX src
  FILES 
  ${IMGUI_DOCKING_DIR}/imgui.cpp
  ${IMGUI_DOCKING_DIR}/imgui_demo.cpp
  ${IMGUI_DOCKING_DIR}/imgui_draw.cpp
  ${IMGUI_DOCKING_DIR}/imgui_tables.cpp
  ${IMGUI_DOCKING_DIR}/imgui_widgets.cpp
  ${IMGUI_DOCKING_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_DOCKING_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DOCKING_DIR}/backends/imgui_impl_vulkan.cpp
)
# library target 
add_library(${CURRENT_MODULE_NAME} SHARED 
  ${IMGUI_DOCKING_DIR}/imgui.cpp
  ${IMGUI_DOCKING_DIR}/imgui_demo.cpp
  ${IMGUI_DOCKING_DIR}/imgui_draw.cpp
  ${IMGUI_DOCKING_DIR}/imgui_tables.cpp
  ${IMGUI_DOCKING_DIR}/imgui_widgets.cpp
  ${IMGUI_DOCKING_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_DOCKING_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DOCKING_DIR}/backends/imgui_impl_vulkan.cpp
)
# include directories
target_include_directories(${CURRENT_MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../ ${IMGUI_DOCKING_DIR})
# link libraries
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC XCore)
find_package(Vulkan REQUIRED)
target_link_libraries(${CURRENT_MODULE_NAME} PUBLIC Vulkan::Vulkan)
# export
include(GenerateExportHeader)
string(TOUPPER ${CURRENT_MODULE_NAME} CK_MODULE_NAME_UPPER)
generate_export_header(${CURRENT_MODULE_NAME}
    EXPORT_MACRO_NAME ${CK_MODULE_NAME_UPPER}_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME}Export.h
)
target_compile_definitions(${CURRENT_MODULE_NAME} PUBLIC -DIMGUI_API=${CK_MODULE_NAME_UPPER}_API)
# precompile header
target_precompile_headers(${CURRENT_MODULE_NAME}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_MODULE_NAME}Export.h
)
