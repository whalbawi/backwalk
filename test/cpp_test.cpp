// NOLINTBEGIN(misc-use-internal-linkage, cppcoreguidelines-pro-type-vararg)
#include <cxxabi.h>             // for __cxa_demangle, abi
#include <stdlib.h>             // for free

#include <cstdint>              // for uintptr_t

#include <functional>           // for function
#include <vector>               // for vector

#include "common.h"             // for BW_UNUSED
#include "backwalk/backwalk.h"  // for bw_backtrace

#include "test.h"               // for TEST, TEST_RUN, TEST_ASSERT_EQ_INT32

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

TEST(backtrace, {
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

TEST(lambda_in_callstack, {
    int x = 0;
    auto lambda = [&x]() { return bw_backtrace(increment_backtrace_cb, &x); };

    const auto retval = lambda_caller(lambda);
    TEST_ASSERT_TRUE(retval);
    TEST_ASSERT_GE_INT32(x, 2);
})

TEST(lambda_cb, {
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

TEST(demangle, {
    auto lambda = [](uintptr_t, const char*, const char* sname, void* arg) {
        auto* fail = static_cast<int*>(arg);
        auto* demangled = abi::__cxa_demangle(sname, nullptr, nullptr, fail);
        BW_UNUSED(demangled);

        return false;
    };

    int fail = 1;
    auto retval = bw_backtrace(lambda, &fail);
    TEST_ASSERT_FALSE(retval);
    TEST_ASSERT_EQ_INT32(fail, 0);
})

std::vector<uintptr_t> addrs;
bool vector_collect(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(fname);
    BW_UNUSED(sname);
    auto* vec = static_cast<std::vector<uintptr_t>*>(arg);
    vec->push_back(addr);
    return true;
}

} // namespace test_ns

namespace other_ns {

TEST(cross_namespace, {
    auto retval = bw_backtrace(test_ns::vector_collect, &test_ns::addrs);
    TEST_ASSERT_TRUE(retval);
    TEST_ASSERT_GE_SIZE(test_ns::addrs.size(), 1L);
})

} // namespace other_ns

extern "C" {
char* bw_dbg_demangle(const char* sname) {
    int fail = 1;
    auto* demangled = abi::__cxa_demangle(sname, nullptr, nullptr, &fail);
    return fail == 0 ? demangled : nullptr;
}

void bw_dbg_demangle_free(char* sname) {
    free(sname); // NOLINT(cppcoreguidelines-no-malloc)
}
}

int main(int argc, char** argv) {
    TEST_INIT("cpp", argc, argv);

    TEST_RUN(test_ns::backtrace);
    TEST_RUN(test_ns::lambda_in_callstack);
    TEST_RUN(test_ns::lambda_cb);
    TEST_RUN(test_ns::demangle);
    TEST_RUN(other_ns::cross_namespace);

    TEST_EXIT();
}

// NOLINTEND(misc-use-internal-linkage, cppcoreguidelines-pro-type-vararg)
