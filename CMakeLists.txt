cmake_minimum_required(VERSION 3.20)
project(backwalk C CXX ASM)

enable_testing()

add_compile_options(
    -fno-omit-frame-pointer
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -g
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directories
set(BACKWALK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BACKWALK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BACKWALK_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Project version
file(READ VERSION BACKWALK_PROJECT_VERSION)
string(STRIP ${BACKWALK_PROJECT_VERSION} BACKWALK_PROJECT_VERSION)

# and commit
execute_process(COMMAND git describe --always --dirty
    OUTPUT_VARIABLE BACKWALK_GIT_COMMIT_HASH
    ERROR_VARIABLE BACKWALK_GIT_COMMIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE BACKWALK_GIT_COMMIT_RESULT
)
if (NOT ${BACKWALK_GIT_COMMIT_RESULT} STREQUAL 0)
    message(WARNING "Could not retrieve git commit: " ${BACKWALK_GIT_COMMIT_ERROR})
    set(BACKWALK_GIT_COMMIT_HASH unknown)
endif()

message("backwalk version information: " ${BACKWALK_PROJECT_VERSION} " - " ${BACKWALK_GIT_COMMIT_HASH})
configure_file(${BACKWALK_SRC_DIR}/version.c.in ${CMAKE_CURRENT_BINARY_DIR}/gen/version.c @ONLY)

# Source files
set(BACKWALK_SRC_LIST
    ${BACKWALK_SRC_DIR}/backwalk.c
    ${BACKWALK_SRC_DIR}/context.c
    ${BACKWALK_SRC_DIR}/asm/context_aarch64.S
    ${BACKWALK_SRC_DIR}/asm/context_x64.S
    ${CMAKE_CURRENT_BINARY_DIR}/gen/version.c
)

file(GLOB_RECURSE ASM_FILES "${BACKWALK_SRC_DIR}/asm/*")
message(${ASM_FILES})
if (BW_IWYU)
    list(REMOVE_ITEM BACKWALK_SRC_LIST ${ASM_FILES})
endif()


add_library(backwalk ${BACKWALK_SRC_LIST})
target_include_directories(backwalk PUBLIC ${BACKWALK_INCLUDE_DIR})
if (BW_DEBUG_ENABLED)
    target_compile_definitions(backwalk PRIVATE BW_DEBUG_ENABLED)
endif()

function(bw_test TEST_NAME)
    add_executable(${TEST_NAME} ${BACKWALK_TEST_DIR}/${TEST_NAME}.c)
    target_include_directories(${TEST_NAME} PRIVATE ${BACKWALK_SRC_DIR} ${BACKWALK_TEST_DIR})
    target_link_libraries(${TEST_NAME} backwalk)
    target_link_options(${TEST_NAME} PRIVATE -rdynamic)
    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

bw_test(noinline_test)
target_compile_options(noinline_test BEFORE PRIVATE -fno-inline -fno-optimize-sibling-calls)
bw_test(version_test)
bw_test(backtrace_test)
bw_test(edge_cases_test)
target_compile_options(edge_cases_test BEFORE PRIVATE -fno-optimize-sibling-calls)
bw_test(stress_test)
bw_test(threading_test)
target_link_libraries(threading_test pthread)


file(GLOB_RECURSE 
    HDR_FILES
    "${BACKWALK_SRC_DIR}/*.h"
    "${BACKWALK_INCLUDE_DIR}/*.h"
    "${BACKWALK_TEST_DIR}/*.h"
)
add_custom_target(lint
  COMMAND /usr/bin/clang-tidy -p "${CMAKE_BINARY_DIR}"  ${HDR_FILES}
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-clang-tidy.py -allow-no-checks -p=${CMAKE_BINARY_DIR}
  COMMENT "Running clang-tidy..."
)

add_custom_target(fmt
  COMMAND git clang-format --diff
  COMMENT "Running clang-format..."
)

add_custom_target(iwyu
  COMMAND iwyu_tool -p ${CMAKE_BINARY_DIR} --
  -Xiwyu --check_also=${BACKWALK_SRC_DIR}/*.h
  -Xiwyu --check_also=${BACKWALK_TEST_DIR}/*.h
  -Xiwyu --update_comments
  -Xiwyu --error=1
  COMMENT "Running iwyu..."
)

add_custom_target(iwyu-fix
  COMMAND iwyu_tool -p ${CMAKE_BINARY_DIR} --
  -Xiwyu --check_also=${BACKWALK_SRC_DIR}/*.h
  -Xiwyu --check_also=${BACKWALK_TEST_DIR}/*.h
  -Xiwyu --update_comments
  | fix_include --update_comments --nosafe_headers --noreorder
  COMMENT "Running iwyu and applying suggestions..."
)