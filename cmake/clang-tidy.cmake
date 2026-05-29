option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

function(enable_clang_tidy target)
    if(NOT ENABLE_CLANG_TIDY)
        return()
    endif()

    if(NOT CLANG_TIDY_BIN)
        find_program(CLANG_TIDY_BIN NAMES "clang-tidy" REQUIRED)
    endif()

    if(NOT CLANG_TIDY_BIN)
        message(WARNING "clang-tidy not found.")
    endif()

    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_BIN}"
        "--allow-no-checks"
        "--quiet"
        "--extra-arg=--target=${PICO_GCC_TRIPLE}"
    )

    # Sets pico-sdk header as system headers to avoid linking
    set(_implicit_includes
        ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
        ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}
    )
    list(REMOVE_DUPLICATES _implicit_includes)
    foreach(include ${_implicit_includes})
        list(APPEND CLANG_TIDY_COMMAND "--extra-arg=-isystem${include}")
    endforeach()

    set_source_files_properties(${CMAKE_BINARY_DIR}/_deps/pico_sdk-src/src/rp2_common/pico_cxx_options/new_delete.cpp PROPERTIES
        SKIP_LINTING ON
    )

    set_target_properties("${target}" PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
    )
endfunction()