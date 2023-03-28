#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <camkes.h>
#include <utils/util.h>

#include <top_types.h>

#define lock() \
    do { \
        if (encrypt_lock()) { \
            printf("[%s] failed to lock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

#define unlock() \
    do { \
        if (encrypt_unlock()) { \
            printf("[%s] failed to unlock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

// From UART to Encrypt
static queue_t recv_queue;

// From Encrypt to Telemetry
static queue_t send_queue;

void pre_init() {
    LOG_ERROR("Starting Encrypt");
    LOG_ERROR("In pre_init");

    queue_init(&recv_queue);
    queue_init(&send_queue);

    LOG_ERROR("Out pre_init");
}

// Simulate encryption
// No encryption now, so just give FC_Data to Telem_Data
// TODO: Implement actual encryption
static void Encrypt_FC_Data_to_Telem_Data() {
    lock();

    if (recv_queue.size + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Send queue not enough!");
    }

    for (uint32_t i=0; i < recv_queue.size; i++) {
        uint8_t tmp;
        if (dequeue(&recv_queue, &tmp)) {
            break;
        }
        if (enqueue(&send_queue, tmp)) {
            break;
        }
    }

    unlock();
}

static int send_to_telemetry(void) {

    return 0;
}

static int read_from_uart(void) {

    return 0;
}

static void consume_Encrypt2Telemetry_DataReadyAck_callback(void *in_arg) {
    if (consume_Encrypt2Telemetry_DataReadyAck_reg_callback(&consume_Encrypt2Telemetry_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Encrypt2Telemetry_DataReadyAck callback");
    }
}

static void consume_UART2Encrypt_DataReadyEvent_callback(void *in_arg) {
    if (consume_UART2Encrypt_DataReadyEvent_reg_callback(&consume_UART2Encrypt_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register UART2Encrypt_DataReadyEvent callback");
    }
}

void consume_Encrypt2Telemetry_DataReadyAck__init(void) {
    if (consume_Encrypt2Telemetry_DataReadyAck_reg_callback(&consume_Encrypt2Telemetry_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Encrypt2Telemetry_DataReadyAck callback");
    }
}

void consume_UART2Encrypt_DataReadyEvent__init(void) {
    if (consume_UART2Encrypt_DataReadyEvent_reg_callback(&consume_UART2Encrypt_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register UART2Encrypt_DataReadyEvent callback");
    }
}

int run() {
    LOG_ERROR("In run");
    while (1) {

    }

    return 0;
}
