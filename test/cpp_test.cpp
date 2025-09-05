// NOLINTBEGIN(misc-use-internal-linkage, cppcoreguidelines-pro-type-vararg)
#include <cxxabi.h>             // for __cxa_demangle, abi

#include <cstdint>              // for uintptr_t

#include <functional>           // for function

#include "common.h"             // for BW_UNUSED
#include "backwalk/backwalk.h"  // for bw_backtrace

#include "test.h"               // for TEST, TEST_ASSERT_EQ_INT32, TEST_RUN

namespace test_ns {

typedef struct {
    int fnum;
    int addr_ok;
    int fname_ok;
    int sname_ok;
} ns_backtrace_ctx_t;

__attribute__((noinline)) bool ns_backtrace_cb(uintptr_t addr,
                                               const char* fname,
                                               const char* sname,
                                               void* arg) {
    auto* ctx = static_cast<ns_backtrace_ctx_t*>(arg);
    ctx->addr_ok += addr != 0 ? 1 : 0;
    ctx->fname_ok += fname != nullptr && *fname != '\0' ? 1 : 0;
    ctx->sname_ok += sname != nullptr && *sname != '\0' ? 1 : 0;
    ctx->fnum++;

    return true;
}

__attribute__((noinline)) bool inner_function(ns_backtrace_ctx_t& ctx) {
    return bw_backtrace(ns_backtrace_cb, &ctx);
}

TEST(test_backtrace, {
    auto ctx = ns_backtrace_ctx_t{};

    auto retval = inner_function(ctx);
    TEST_ASSERT_TRUE(retval);

    TEST_ASSERT_GE_INT32(ctx.fnum, 1);
    TEST_ASSERT_EQ_INT32(ctx.addr_ok, ctx.fnum);
    TEST_ASSERT_EQ_INT32(ctx.fname_ok, ctx.fnum);
    TEST_ASSERT_EQ_INT32(ctx.sname_ok, ctx.fnum);
})

__attribute__((noinline)) bool lambda_caller(const std::function<bool()>& lambda) {
    return lambda();
}

__attribute__((noinline)) bool increment_backtrace_cb(uintptr_t addr,
                                                      const char* fname,
                                                      const char* sname,
                                                      void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);
    auto* fnum = static_cast<int*>(arg);
    *fnum += 1;

    return true;
}

TEST(test_lambda_in_callstack, {
    int x = 0;
    auto lambda = [&x]() { return bw_backtrace(increment_backtrace_cb, &x); };

    const auto retval = lambda_caller(lambda);
    TEST_ASSERT_TRUE(retval);
    TEST_ASSERT_GE_INT32(x, 2);
})

TEST(test_lambda_cb, {
    int fnum = 0;
    auto lambda = [](uintptr_t, const char*, const char*, void* arg) {
        int* fnum = static_cast<int*>(arg);
        *fnum += 1;
        return true;
    };

    auto retval = bw_backtrace(lambda, &fnum);
    TEST_ASSERT_TRUE(retval);
    TEST_ASSERT_GE_INT32(fnum, 1);
})

TEST(test_demangle, {
    auto lambda = [](uintptr_t, const char*, const char* sname, void* arg) {
        auto* demangle_fail = static_cast<int*>(arg);
        auto* demangled = abi::__cxa_demangle(sname, nullptr, nullptr, demangle_fail);
        BW_UNUSED(demangled);

        return false;
    };

    int demangle_fail = 1;
    auto retval = bw_backtrace(lambda, &demangle_fail);
    TEST_ASSERT_FALSE(retval);
    TEST_ASSERT_EQ_INT32(demangle_fail, 0);
})

} // namespace test_ns

int main(void) {
    TEST_INIT("cpp");

    TEST_RUN(test_ns::test_backtrace);
    TEST_RUN(test_ns::test_lambda_in_callstack);
    TEST_RUN(test_ns::test_lambda_cb);
    TEST_RUN(test_ns::test_demangle);

    TEST_EXIT();
}

// NOLINTEND(misc-use-internal-linkage, cppcoreguidelines-pro-type-vararg)
