#include "common.h"

void ticket_init(ticket_t* ticket) {
    ticket->cv = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    ticket->mtx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    ticket->state = false;
}

#define MUTEX_PROTECT(mutex, body)                                                                 \
    BW_UNUSED(pthread_mutex_lock(mutex));                                                          \
    do {                                                                                           \
        body;                                                                                      \
    } while (0);                                                                                   \
    BW_UNUSED(pthread_mutex_unlock(mutex));

void ticket_signal(ticket_t* ticket) {
    MUTEX_PROTECT(&ticket->mtx, ticket->state = true);
    BW_UNUSED(pthread_cond_signal(&ticket->cv));
}

void ticket_wait(ticket_t* ticket) {
    MUTEX_PROTECT(&ticket->mtx, {
        while (!ticket->state) {
            BW_UNUSED(pthread_cond_wait(&ticket->cv, &ticket->mtx));
        }
    });
}
