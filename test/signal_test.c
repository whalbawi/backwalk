#include <errno.h>              // for errno
#include <pthread.h>            // for pthread_create, pthread_join, pthread_t
#include <signal.h>             // for size_t, raise, signal, SIGUSR1, SIG_ERR
#include <stdbool.h>            // for bool, true
#include <stdint.h>             // for uint8_t, uintptr_t
#include <unistd.h>             // for _exit, NULL, close, pipe, read, write

#include "common.h"             // for BW_UNUSED, ticket_init, ticket_signal
#include "backwalk/backwalk.h"  // for bw_backtrace_collect, bw_backtrace_pr...

#include "test.h"               // for TEST_ERROR_NONZERO, TEST_RESULT_ERR

// _exit is async-signal-safe. See: https://man7.org/linux/man-pages/man7/signal-safety.7.html
#define EXIT_IF(cond)                                                                              \
    if ((cond) != 0) {                                                                             \
        _exit(TEST_RESULT_ERR);                                                                    \
    }

#define EXIT_ON_NONZERO(f) EXIT_IF((f) != 0)

typedef struct {
    bw_context_t* backtrace_ctx;
    int bt_collect_pipe[2];
    pthread_t worker_th;
    ticket_t worker_ready;
} test_state_t;

test_state_t test_state;

static bool backtrace_cb(uintptr_t addr, const char* fname, const char* sname, void* arg) {
    BW_UNUSED(addr);
    BW_UNUSED(fname);
    BW_UNUSED(sname);

    size_t* frame_cnt = (size_t*)arg;
    ++*frame_cnt;

    return true;
}

void* backtrace_processor(void* arg) {
    ticket_signal(&test_state.worker_ready);

    uint8_t signal = 0;
    // Wait for the signal handler's notification that the backtrace is ready for processing
    EXIT_IF(read(test_state.bt_collect_pipe[0], &signal, sizeof(signal)) < 0);

    EXIT_IF(!bw_backtrace_process(test_state.backtrace_ctx, backtrace_cb, arg));

    return NULL;
}

void signal_handler(int signo) {
    BW_UNUSED(signo);

    // NOLINTNEXTLINE(bugprone-signal-handler)
    EXIT_IF(!bw_backtrace_collect(test_state.backtrace_ctx));

    uint8_t signal = 1;
    EXIT_IF(write(test_state.bt_collect_pipe[1], &signal, sizeof(signal)) < 0);
}

__attribute__((noinline)) int raise_signal(int signo) {
    if (raise(signo) != 0) {
        return 1;
    }

    return 0;
}

#pragma GCC diagnostic push
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma GCC diagnostic ignored "-Wc23-extensions"
#endif
TEST(safe_backtrace, {
    TEST_ERROR_NONZERO(pipe(test_state.bt_collect_pipe));

    ticket_init(&test_state.worker_ready);
    size_t frame_cnt = 0;

    TEST_ERROR_NONZERO(
        pthread_create(&test_state.worker_th, NULL, backtrace_processor, &frame_cnt));

    ticket_wait(&test_state.worker_ready);

    test_state.backtrace_ctx = mkbw_context(16);
    if (test_state.backtrace_ctx == NULL) {
        // HACK: Not passing a variadic argument causes a C23-extension warning
        TEST_ERROR("could not create backwalk context");
    }

    int signo = SIGUSR1;
    if (signal(signo, signal_handler) == SIG_ERR) {
        TEST_ERROR("could not install signal handler. errno=%d", errno);
    }

    TEST_ERROR_NONZERO(raise_signal(signo));

    TEST_ERROR_NONZERO(pthread_join(test_state.worker_th, NULL));

    TEST_ASSERT_GE_SIZE(frame_cnt, 1UL);
    bw_context_fini(test_state.backtrace_ctx);
    BW_UNUSED(close(test_state.bt_collect_pipe[1]));
    BW_UNUSED(close(test_state.bt_collect_pipe[0]));
})
#pragma GCC diagnostic pop

int main(int argc, char** argv) {
    TEST_INIT("signal", argc, argv);

    TEST_RUN(safe_backtrace);

    TEST_EXIT();
}
