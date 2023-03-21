#include <stdint.h>
#include <string.h>
#include <camkes.h>
#include <stdlib.h>
#include "Decrypt.h"
#include "smaccm_types.h"

static bool occurred_periodic_dispatcher;
static int64_t time_periodic_dispatcher;

bool periodic_dispatcher_write_int64_t(const int64_t *arg) {
    occurred_periodic_dispatcher = true;
    time_periodic_dispatcher = *arg;
    MUTEXOP(dispatch_sem_post());
    return true;
}

void timer_complete_callback(void *_ UNUSED) {
   // we want time in microseconds, not nanoseconds, so we divide by 1000.
   int64_t time_periodic_dispatcher = timer_time() / 1000LL;
   (void)periodic_dispatcher_write_int64_t(&time_periodic_dispatcher);
   CALLBACKOP(timer_complete_reg_callback(timer_complete_callback, NULL));
}

bool self2server_enqueue(const SMACCM_DATA__GIDL_container * tb_self2server) {
    return self2server0_enqueue((SMACCM_DATA__GIDL_container *)tb_self2server);
}

void component_init(const int64_t *n_var0) {
    printf("Decrypt init\n");
    callback_configureStaticKey(n_var0);
}

void component_entry_aux(void) {
    // SMACCM_DATA__UART_Packet_i n_local0;
    // if (uart2self_dequeue(&n_local0)) {
    //     printf("UART -> Decrypt\n");
    //     callback_input_uart2self_dequeue_handler(&n_local0);
    // } else {
    //     printf("No data for Decrypt\n");
    // }
}

void component_entry(const int64_t *n_var0)
{
    printf("Decrypt\n");
    component_entry_aux();

    callback_periodic_pop(n_var0);

    return;
}

void entrypoint_Decrypt_periodic_dispatcher(const int64_t * in_arg) {
    component_entry((int64_t *) in_arg);
}

void entrypoint_Decrypt_initializer(const int64_t *in) {
    component_init(in);
}

int run(void) {

    // Port initialization routines

    // tb_timer_periodic(0, ((uint64_t)1)*NS_IN_MS);
    CALLBACKOP(timer_complete_reg_callback(timer_complete_callback, NULL));

    int64_t key;
    entrypoint_Decrypt_initializer(&key);

    // Initial lock to await dispatch input.
    MUTEXOP(dispatch_sem_wait());
    while (1) {
        MUTEXOP(dispatch_sem_wait());

        // Drain the queues
        if (occurred_periodic_dispatcher) {
            occurred_periodic_dispatcher = false;
            entrypoint_Decrypt_periodic_dispatcher(&time_periodic_dispatcher);
        }

    }

    // Won't ever get here, but form must be followed
    return 0;
}
