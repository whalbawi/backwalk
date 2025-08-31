cmake_minimum_required(VERSION 3.20)
project(skeleton CXX)

set(CMAKE_CXX_STANDARD 14)

enable_testing()

# Directories
set(SKELETON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SKELETON_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(SKELETON_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(SKELETON_GOOGLETEST_DIR ${SKELETON_THIRD_PARTY_DIR}/googletest)

# Dependencies
add_subdirectory(${SKELETON_GOOGLETEST_DIR} EXCLUDE_FROM_ALL)

# Project version
file(READ VERSION SKELETON_PROJECT_VERSION)
string(STRIP ${SKELETON_PROJECT_VERSION} SKELETON_PROJECT_VERSION)

# and commit
execute_process(COMMAND git describe --always --dirty
    OUTPUT_VARIABLE SKELETON_GIT_COMMIT_HASH
    ERROR_VARIABLE SKELETON_GIT_COMMIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE SKELETON_GIT_COMMIT_RESULT
)
if (NOT ${SKELETON_GIT_COMMIT_RESULT} STREQUAL 0)
    message(WARNING "Could not retrieve git commit: " ${SKELETON_GIT_COMMIT_ERROR})
    set(SKELETON_GIT_COMMIT_HASH unknown)
endif()

message("skeleton version information: " ${SKELETON_PROJECT_VERSION} " - " ${SKELETON_GIT_COMMIT_HASH})
configure_file(${SKELETON_SRC_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/gen/version.h @ONLY)

# Source files
set(SKELETON_SRC_LIST
    ${SKELETON_SRC_DIR}/skeleton.cpp
)

# Test files
set(SKELETON_TEST_LIST
    ${SKELETON_TEST_DIR}/skeleton_test.cpp
)

add_library(skeleton-lib ${SKELETON_SRC_LIST})
target_include_directories(skeleton-lib PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)
set_target_properties(skeleton-lib PROPERTIES OUTPUT_NAME skeleton)

add_executable(skeleton-tests ${SKELETON_TEST_LIST})
target_include_directories(skeleton-tests PRIVATE ${SKELETON_SRC_DIR})
target_link_libraries(skeleton-tests skeleton-lib gtest_main)

add_test(unit-tests skeleton-tests)
