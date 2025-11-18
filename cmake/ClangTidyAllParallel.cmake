# ClangTidyAllParallel.cmake
# Parallel clang-tidy sweep (always-run per file targets).
#
# Simplified: only an always-run mode remains. Each source file gets its own
# custom target so the build system can schedule them in parallel (e.g. ninja -j).
# Invoking the aggregate target re-runs analysis for all translation units.
#
# Expected cache variables from including scope:
#   CLANG_TIDY_EXE_ALL            : path to clang-tidy executable
#   CLANG_TIDY_ALL_HEADER_FILTER  : header filter regex
#   CLANG_TIDY_ALL_EXTRA_ARGS     : extra args string (may be empty)
#   CLANG_TIDY_ALL_FAIL_ON_ERROR  : ON/OFF to treat non‑zero exit as fatal
#   CMAKE_SOURCE_DIR / CMAKE_BINARY_DIR : standard CMake vars
# Defines aggregate target: clang_tidy_all

option(CLANG_TIDY_ALL_SHOW_FRONTEND_WARNINGS "Do NOT suppress Clang front-end warnings (omit -extra-arg=-w)" OFF)
option(CLANG_TIDY_ALL_SUPPRESS_SUMMARY "Strip the repeated '<n> warnings generated.' lines from clang front-end output" ON)
if(NOT DEFINED CLANG_TIDY_ALL_HEADER_FILTER OR CLANG_TIDY_ALL_HEADER_FILTER STREQUAL "")
    # Default to all project-owned headers (non-system) under common source roots.
    set(CLANG_TIDY_ALL_HEADER_FILTER "${CMAKE_SOURCE_DIR}/(src|include|tests)/.*")
endif()

# Compose extra args, optionally adding suppression flag if user turns warnings off.
set(_clang_tidy_all_cmd_extra "${CLANG_TIDY_ALL_EXTRA_ARGS}")
if(NOT CLANG_TIDY_ALL_SHOW_FRONTEND_WARNINGS)
    if(NOT _clang_tidy_all_cmd_extra MATCHES "-extra-arg=-w")
        if(NOT "${_clang_tidy_all_cmd_extra}" STREQUAL "")
            set(_clang_tidy_all_cmd_extra "${_clang_tidy_all_cmd_extra} -extra-arg=-w")
        else()
            set(_clang_tidy_all_cmd_extra "-extra-arg=-w")
        endif()
    endif()
endif()

# Build the base invocation (shared between fatal/non-fatal variants). We add a pipe
# to sed to filter summary-only lines produced by the clang front-end when parsing
# multiple compile commands for the same TU under multi-config generators (e.g. Ninja Multi-Config).
set(_clang_tidy_all_base "${CLANG_TIDY_EXE_ALL} --quiet -p ${CMAKE_BINARY_DIR} -header-filter='${CLANG_TIDY_ALL_HEADER_FILTER}' ${_clang_tidy_all_cmd_extra}")
if(CLANG_TIDY_ALL_SUPPRESS_SUMMARY)
    # Filter summary lines (appear on stderr). We'll redirect stderr to stdout before sed.
    set(_clang_tidy_all_filter " 2>&1 | sed -E '/^[0-9]+ warnings generated\\.$/d'")
else()
    set(_clang_tidy_all_filter "")
endif()

if(NOT CLANG_TIDY_EXE_ALL)
    message(FATAL_ERROR "ClangTidyAllParallel included without CLANG_TIDY_EXE_ALL set")
endif()

file(GLOB_RECURSE CLANG_TIDY_ALL_SOURCES
    LIST_DIRECTORIES OFF
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cc
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.cxx
    ${CMAKE_SOURCE_DIR}/tests/*.cc
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/*.cxx
)
list(LENGTH CLANG_TIDY_ALL_SOURCES _tidy_src_count)
message(STATUS "Configuring parallel clang-tidy-all for ${_tidy_src_count} translation units")

set(_clang_tidy_all_targets)
foreach(_src IN LISTS CLANG_TIDY_ALL_SOURCES)
    get_filename_component(_src_name ${_src} NAME)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _tidy_target_name "${_src_name}")
    set(_tidy_target "clang_tidy_file_${_tidy_target_name}")
    if(CLANG_TIDY_ALL_FAIL_ON_ERROR)
        add_custom_target(${_tidy_target}
            COMMAND /bin/sh -c "${_clang_tidy_all_base} ${_src}${_clang_tidy_all_filter}"
            COMMENT "clang-tidy: ${_src}"
            VERBATIM
        )
    else()
        # Non‑fatal: swallow non‑zero exit status.
        add_custom_target(${_tidy_target}
            COMMAND /bin/sh -c "${_clang_tidy_all_base} ${_src}${_clang_tidy_all_filter} || true"
            COMMENT "clang-tidy (non-fatal): ${_src}"
            VERBATIM
        )
    endif()
    list(APPEND _clang_tidy_all_targets ${_tidy_target})
endforeach()
add_custom_target(clang_tidy_all DEPENDS ${_clang_tidy_all_targets})
set_property(TARGET clang_tidy_all PROPERTY COMMENT "Running clang-tidy across all sources (parallel always-run) - use -j for concurrency")
