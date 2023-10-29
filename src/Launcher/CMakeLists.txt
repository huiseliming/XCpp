file(GLOB_RECURSE XLauncher_HEADERS *.h *.hh *.hpp *.hxx)
file(GLOB_RECURSE XLauncher_SOURCES *.c *.cc *.cpp *.cxx)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR} 
  PREFIX src
  FILES ${XLauncher_HEADERS} ${XLauncher_SOURCES}
)

add_executable(XLauncher ${XLauncher_HEADERS} ${XLauncher_SOURCES})
target_link_libraries(XLauncher PRIVATE XCore XRenderer XImGui XApp)
