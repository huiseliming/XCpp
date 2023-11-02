if(NOT EXISTS "${CMAKE_BINARY_DIR}/compile_flags.txt")
    write_file(${CMAKE_BINARY_DIR}/compile_flags.txt "-xc++")
endif()

set(CPP_KIT_HEADER_TOOL_VER "v0.0.0")
if(NOT EXISTS ${CMAKE_BINARY_DIR}/XCppCodeGenerator.exe)
    file(DOWNLOAD
        "https://github.com/huiseliming/XCppCodeGenerator/releases/download/v0.1.0/XCppCodeGenerator.rar"
        ${CMAKE_BINARY_DIR}/XCppCodeGenerator.rar
        SHOW_PROGRESS
    )
    execute_process(COMMAND
        ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/XCppCodeGenerator.rar
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
set(XCPP_CODE_GENERATOR_EXECUTABLE ${CMAKE_BINARY_DIR}/XCppCodeGenerator.exe CACHE FILEPATH "XCppCodeGenerator executable path")

macro(xcpp_code_generator CPP_TARGET_NAME)
    set(${CPP_TARGET_NAME}_HEADERS_ALL_CPP ${CMAKE_BINARY_DIR}/${CPP_TARGET_NAME}.all.cpp)
    file(WRITE ${${CPP_TARGET_NAME}_HEADERS_ALL_CPP} "")
    foreach(HeaderFile ${${CPP_TARGET_NAME}_HEADERS})
        file(APPEND ${${CPP_TARGET_NAME}_HEADERS_ALL_CPP} "#include \"${HeaderFile}\"\n")
    endforeach()
    set(${CPP_TARGET_NAME}_GENERATED_HEADERS "")
    set(${CPP_TARGET_NAME}_GENERATED_SOURCES "")
    foreach(HEADER_FILE ${${CPP_TARGET_NAME}_HEADERS})
        file(RELATIVE_PATH HEADER_FILE_RELATIVE_PATH ${CMAKE_SOURCE_DIR} ${HEADER_FILE})
        string(REGEX REPLACE "\\.[^.]*$" "" GENERATED_RELATIVE_PATH ${HEADER_FILE_RELATIVE_PATH})
        list(APPEND ${CPP_TARGET_NAME}_GENERATED_HEADERS "${CMAKE_BINARY_DIR}/${GENERATED_RELATIVE_PATH}.gen.h")
        list(APPEND ${CPP_TARGET_NAME}_GENERATED_SOURCES "${CMAKE_BINARY_DIR}/${GENERATED_RELATIVE_PATH}.gen.cpp")
    endforeach()
    list(JOIN ${CPP_TARGET_NAME}_HEADERS "\" --target=\"" EXTRA_ARG_HEADERS)
    add_custom_target(${CPP_TARGET_NAME}-CodeGenerator
        COMMAND ${XCPP_CODE_GENERATOR_EXECUTABLE}
        ${${CPP_TARGET_NAME}_HEADERS_ALL_CPP} 
        --src_dir=${CMAKE_SOURCE_DIR} 
        --gen_dir=${CMAKE_BINARY_DIR} 
        --target="${EXTRA_ARG_HEADERS}" 
        -- clang++ -Xclang -std=c++20 -fexceptions -Wno-everything -D__XCPP_CODE_GENERATOR__ 
        "-I\"$<JOIN:$<TARGET_PROPERTY:${CPP_TARGET_NAME},INCLUDE_DIRECTORIES>,\" -I\">\""
        "-include\"$<JOIN:$<TARGET_PROPERTY:${CPP_TARGET_NAME},PRECOMPILE_HEADERS>,\" -include\">\""
        "-D$<JOIN:$<TARGET_PROPERTY:${CPP_TARGET_NAME},COMPILE_DEFINITIONS>, -D>" 
        -c
        #DEPENDS ${${CPP_TARGET_NAME}_HEADERS}
        BYPRODUCTS ${${CPP_TARGET_NAME}_GENERATED_HEADERS} ${${CPP_TARGET_NAME}_GENERATED_SOURCES}
        COMMAND_EXPAND_LISTS
    )
    target_sources(${CPP_TARGET_NAME} PRIVATE ${${CPP_TARGET_NAME}_GENERATED_HEADERS} ${${CPP_TARGET_NAME}_GENERATED_SOURCES})
    source_group(
        TREE ${CMAKE_BINARY_DIR}
        PREFIX gen
        FILES ${${CPP_TARGET_NAME}_GENERATED_HEADERS} ${${CPP_TARGET_NAME}_GENERATED_SOURCES}
    )
    #set_property(TARGET ${CPP_TARGET_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${${CPP_TARGET_NAME}_GENERATED_HEADERS} ${${CPP_TARGET_NAME}_GENERATED_SOURCES})
    add_dependencies(${CPP_TARGET_NAME} ${CPP_TARGET_NAME}-CodeGenerator)
endmacro()

