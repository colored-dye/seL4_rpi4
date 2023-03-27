#include "camkes-component-echo.h"
#include "utils/attribute.h"
#include "utils/util.h"
#include <cstdlib>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <camkes.h>

#include <top_types.h>

#define lock() \
    do { \
        if (decrypt_lock()) { \
            printf("[%s] failed to lock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

#define unlock() \
    do { \
        if (decrypt_unlock()) { \
            printf("[%s] failed to unlock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

// From Telemetry to Decrypt
static queue_t recv_queue;

// From Decrypt to UART
static queue_t send_queue;

void pre_init() {
    LOG_ERROR("Starting Decrypt");
    LOG_ERROR("In pre_init");

    queue_init(&recv_queue);
    queue_init(&send_queue);

    LOG_ERROR("Out pre_init");
}

// Simulate decryption
// No encryption now, so just give Telem_Data to FC_Data
// TODO: Implement actual decryption
static void Decrypt_Telem_Data_to_FC_Data() {
    lock();

    if (recv_queue.size + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Send queue not enough!");
    }

    for (int i=0; i < recv_queue.size; i++) {
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

static int send_to_UART() {
    // Protect send_queue
    lock();

    if (queue_empty(&send_queue)) {
        unlock();
        return -1;
    }

    Telem_Data *telem_data = (Telem_Data *) send_FC_Data_Decrypt2UART;
    int size = send_queue.size;
    for (int i=0; i < size; i++) {
        uint8_t tmp;
        dequeue(&send_queue, &tmp);
        telem_data->raw_data[i] = tmp;
        send_FC_Data_Decrypt2UART_release();
    }
    telem_data->len = size;

    emit_Decrypt2UART_DataReady_emit();

    unlock();
}

static void consume_Telemetry2Decrypt_DataReadyEvent_callback(void *in_arg UNUSED) {
    
    if (consume_Telemetry2Decrypt_DataReadyEvent_reg_callback(&consume_Telemetry2Decrypt_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyEvent callback");
    }
}

static void consume_Decrypt2UART_DataReadyAck_callback(void *in_arg UNUSED) {
    send_to_UART();

    if (consume_Decrypt2UART_DataReadyAck_reg_callback(&consume_Decrypt2UART_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyAck callback");
    }
}

void consume_Telemetry2Decrypt_DataReadyEvent__init() {
    if (consume_Telemetry2Decrypt_DataReadyEvent_reg_callback(&consume_Telemetry2Decrypt_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyEvent callback");
    }
}

void consume_Decrypt2UART_DataReadyAck__init() {
    if (consume_Decrypt2UART_DataReadyAck_reg_callback(&consume_Decrypt2UART_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyAck callback");
    }
}

int run(void) {
    LOG_ERROR("In run");
    while (1) {
        Decrypt_Telem_Data_to_FC_Data();
    }

    return 0;
}
