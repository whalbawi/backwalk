#ifndef BW_TEST_COMMON_H
#define BW_TEST_COMMON_H

#include <stdio.h>

#include "common.h"

typedef enum {
    TEST_RESULT_OK = 0,
    TEST_RESULT_FAIL = 1,
    TEST_RESULT_ERR = 2,
} test_result_t;

#define TEST_RESULT_ test_result_t

#define TEST_SUITENAME_ tests_suite_name
#define TEST_FINAL_RESULT_ tests_final_result

#define TEST_INIT_(suite_name, final_result)                                                       \
    const char* tests_suite_name = suite_name;                                                     \
    TEST_RESULT_ final_result = TEST_RESULT_OK

#define TEST_FINAL_RESULT_UPDATE_(test_result)                                                     \
    do {                                                                                           \
        if ((test_result) != TEST_RESULT_OK) {                                                     \
            TEST_FINAL_RESULT_ = (test_result);                                                    \
        }                                                                                          \
    } while (0)

#define TEST_EXIT_(final_result)                                                                   \
    do {                                                                                           \
        switch (final_result) {                                                                    \
        case TEST_RESULT_OK:                                                                       \
            return 0;                                                                              \
        case TEST_RESULT_FAIL:                                                                     \
            return 1;                                                                              \
        case TEST_RESULT_ERR:                                                                      \
            return 2;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_INIT(suite_name) TEST_INIT_(suite_name, TEST_FINAL_RESULT_)

#define TEST_EXIT() TEST_EXIT_(TEST_FINAL_RESULT_)

#define TEST_RUN(test_function)                                                                    \
    do {                                                                                           \
        BW_UNUSED(fprintf(stderr, "RUN: %s." #test_function "\n", TEST_SUITENAME_));               \
        TEST_RESULT_ test_result = (test_function)();                                              \
        TEST_FINAL_RESULT_UPDATE_(test_result);                                                    \
        const char* result_str = NULL;                                                             \
        switch (test_result) {                                                                     \
        case TEST_RESULT_OK:                                                                       \
            result_str = "OK";                                                                     \
            break;                                                                                 \
        case TEST_RESULT_FAIL:                                                                     \
            result_str = "FAIL";                                                                   \
            break;                                                                                 \
        case TEST_RESULT_ERR:                                                                      \
            result_str = "ERROR";                                                                  \
            break;                                                                                 \
        default:                                                                                   \
            result_str = "UNKNOWN";                                                                \
        }                                                                                          \
        BW_UNUSED(                                                                                 \
            fprintf(stderr, "END: %s." #test_function " %s\n", TEST_SUITENAME_, result_str));      \
    } while (0)

#define TEST(test_name) TEST_RESULT_ test_name(void)

#define TEST_LOG(fmt, ...)                                                                         \
    do {                                                                                           \
        int res =                                                                                  \
            fprintf(stderr, "%s:%d\n\t" fmt "\n", __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);  \
        BW_UNUSED(res);                                                                            \
    } while (0)

#define TEST_ASSERT_OP_(val, exp, cond, op, fmt)                                                   \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            TEST_LOG("assertion failed: %s " op " %s\n"                                            \
                     "\tactual: " fmt "\n"                                                         \
                     "\texpected: " fmt,                                                           \
                     #val,                                                                         \
                     #exp,                                                                         \
                     (val),                                                                        \
                     (exp));                                                                       \
            return TEST_RESULT_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_EQ_BOOL_(val, exp)                                                             \
    do {                                                                                           \
        if ((val) != (exp)) {                                                                      \
            TEST_LOG("assertion failed: %s == %s\n"                                                \
                     "\tactual: %s \n"                                                             \
                     "\texpected: %s",                                                             \
                     #val,                                                                         \
                     #exp,                                                                         \
                     (val) ? "true" : "false",                                                     \
                     (exp) ? "true" : "false");                                                    \
            return TEST_RESULT_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_TRUE(expr) TEST_ASSERT_EQ_BOOL_(expr, true)

#define TEST_ASSERT_NONNULL(expr)                                                                  \
    do {                                                                                           \
        if ((expr) == NULL) {                                                                      \
            TEST_LOG("assertion failed: %s != NULL", #expr);                                       \
            return TEST_RESULT_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_EQ_(val, exp, fmt) TEST_ASSERT_OP_(val, exp, (val) == (exp), "==", fmt)
#define TEST_ASSERT_EQ_INT(val, exp) TEST_ASSERT_EQ_(val, exp, "%d")
#define TEST_ASSERT_EQ_CHAR(val, exp) TEST_ASSERT_EQ_(val, exp, "%c")

#define TEST_ASSERT_NE_(val, exp, fmt) TEST_ASSERT_OP_(val, exp, (val) != (exp), "!=", fmt)
#define TEST_ASSERT_NE_CHAR(val, exp) TEST_ASSERT_NE_(val, exp, "%c")

#define TEST_ASSERT_GE_INT(val, exp) TEST_ASSERT_OP_(val, exp, (val) >= (exp), ">=", "%d")

#define TEST_OK() return TEST_RESULT_OK

#define TEST_FAIL() return TEST_RESULT_FAIL

#endif // BW_TEST_COMMON_H
