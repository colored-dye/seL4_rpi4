/*
 * CAmkES tutorial part 1: components with RPC. Client part.
 */

#include <stdint.h>
#include <stdio.h>

/* generated header for our component */
#include <camkes.h>

static bool occurred_periodic_dispatcher;
static int64_t time_periodic_dispatcher;

bool periodic_dispatcher_write_in64_t(const int64_t * arg) {
    occurred_periodic_dispatcher = true;
    time_periodic_dispatcher = *arg;
    MUTEXOP(dispatch_sem_post());
}

void timer_complete_callback(void *_ UNUSED) {
    int64_t _time_periodic_dispatcher = timer_time() / 1000LL;
    (void)periodic_dispatcher_write_in64_t(&_time_periodic_dispatcher);
    CALLBACKOP(timer_complete_reg_callback(timer_complete_callback, NULL));
}

void component_entry(const int64_t *n_var0) {
    printf("Encrypt\n");
    // printf("Enter %s:%s\n", __FILE__, __FUNCTION__);
    // printf("Arg: %ld\n", *n_var0);
}

void entrypoint_Encrypt_periodic_dispatcher(const int64_t * in_arg) {
    component_entry((int64_t *) in_arg);
}

void entrypoint_Encrypt_server2self0(const SMACCM_DATA__GIDL *in_arg UNUSED) {
    
}

/* run the control thread */
int run(void) {
    printf("Starting the Encrypt\n");

    CALLBACKOP(timer_complete_reg_callback(timer_complete_callback, NULL));

    // Entrypoint initializer

    // Initial lock to await dispatch input
    MUTEXOP(dispatch_sem_wait());
    while (1) {
        MUTEXOP(dispatch_sem_wait());

        if (occurred_periodic_dispatcher) {
            occurred_periodic_dispatcher = false;
            entrypoint_Encrypt_periodic_dispatcher(&time_periodic_dispatcher);
        }
    }

    printf("Encrypt bye bye~");
    return 0;
}
