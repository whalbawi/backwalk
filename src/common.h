#ifndef BW_COMMON_H
#define BW_COMMON_H

#include <pthread.h>  // for pthread_cond_t, pthread_mutex_t
#ifndef __cplusplus
#include <stdbool.h>  // for bool
#endif

#define BW_UNUSED(expr) (void)(expr)

#define BW_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct {
    pthread_cond_t cv;
    pthread_mutex_t mtx;
    bool state;
} ticket_t;

void ticket_init(ticket_t* ticket);

void ticket_signal(ticket_t* ticket);

void ticket_wait(ticket_t* ticket);

#endif // BW_COMMON_H
