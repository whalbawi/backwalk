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
    ${BACKWALK_SRC_DIR}/asm/context_x64.S
    ${CMAKE_CURRENT_BINARY_DIR}/gen/version.c
)

# Test files
set(BACKWALK_TEST_LIST
    ${BACKWALK_TEST_DIR}/main.c
    ${BACKWALK_TEST_DIR}/version_test.c
)

add_library(backwalk ${BACKWALK_SRC_LIST})
target_include_directories(backwalk PUBLIC ${BACKWALK_INCLUDE_DIR})
if (BW_DEBUG_ENABLED)
    target_compile_definitions(backwalk PRIVATE BW_DEBUG_ENABLED)
endif()

add_executable(backwalk-tests ${BACKWALK_TEST_LIST})
target_include_directories(backwalk-tests PRIVATE ${BACKWALK_SRC_DIR})
target_link_libraries(backwalk-tests backwalk)
target_link_options(backwalk-tests PRIVATE -rdynamic)
add_test(unit-tests backwalk-tests)

function(bw_test TEST_NAME)
    add_executable(${TEST_NAME} ${BACKWALK_TEST_DIR}/${TEST_NAME}.c)
    target_include_directories(${TEST_NAME} PRIVATE ${BACKWALK_SRC_DIR})
    target_link_libraries(${TEST_NAME} backwalk)
    target_link_options(${TEST_NAME} PRIVATE -rdynamic)
    add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

bw_test(noinline_test)
target_compile_options(noinline_test BEFORE PRIVATE -fno-inline -fno-optimize-sibling-calls)


file(GLOB_RECURSE HDR_FILES "${BACKWALK_SRC_DIR}/*.h" "${BACKWALK_INCLUDE_DIR}/*.h")
add_custom_target(lint
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/run-clang-tidy.py -allow-no-checks -p=${CMAKE_BINARY_DIR}
  COMMENT "Running clang-tidy..."
)

add_custom_target(fmt
  COMMAND git clang-format --diff
  COMMENT "Running clang-format..."
)