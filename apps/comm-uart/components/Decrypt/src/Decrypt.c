#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <camkes.h>
#include <utils/util.h>

#include <top_types.h>

#define lock() \
    do { \
        if (decrypt_lock()) { \
            LOG_ERROR("[%s] failed to lock\n", get_instance_name()); \
        } \
    } while (0)

#define unlock() \
    do { \
        if (decrypt_unlock()) { \
            LOG_ERROR("[%s] failed to unlock\n", get_instance_name()); \
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

// Simulate decryption.
// No encryption now, so just give Telem_Data to FC_Data.
// From recv_queue to send_queue
// TODO: Implement actual decryption
static int Decrypt_Telem_Data_to_FC_Data() {
    int error = 0;

    lock();

    if (recv_queue.size + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Send queue not enough!");
    }

    for (uint32_t i=0; i < recv_queue.size; i++) {
        uint8_t tmp;
        if (dequeue(&recv_queue, &tmp)) {
            error = -1;
            break;
        }
        // putchar(tmp);
        if (enqueue(&send_queue, tmp)) {
            error = -1;
            break;
        }
    }

    unlock();

    return error;
}

// Send decrypted FC data to UART
// from send_queue
static int send_to_uart(void) {
    int error = 0;

    uint32_t queue_size;
    while (1) {
        lock();
        queue_size = send_queue.size;
        unlock();
        if (queue_size > 0) {
            break;
        }
    }

    // Protect send_queue
    lock();

    FC_Data *fc_data = (FC_Data *) send_FC_Data_Decrypt2UART;
    uint32_t data_size = send_queue.size;

    if (data_size > sizeof(FC_Data_raw)) {
        data_size = sizeof(FC_Data_raw);
    }

    uint8_t tmp;
    for (uint32_t i=0; i < data_size; i++) {
        if (!dequeue(&send_queue, &tmp)) {
            fc_data->raw_data[i] = tmp;
            send_FC_Data_Decrypt2UART_release();
        } else {
            LOG_ERROR("Should not get here");
            error = -1;
            data_size = 0;
            break;
        }
    }
    fc_data->len = data_size;

    unlock();

    LOG_ERROR("To uart");
    // Decrypt => UART
    emit_Decrypt2UART_DataReadyEvent_emit();

    return error;
}

// Read encrypted data from telemetry
// and push to recv_queue
static int read_from_telemetry(void) {
    int error = 0;
    
    // Protect recv_queue
    lock();

    recv_Telem_Data_Telemetry2Decrypt_acquire();
    Telem_Data *telem_data = (Telem_Data *) recv_Telem_Data_Telemetry2Decrypt;
    uint32_t size = telem_data->len;
    for (uint32_t i = 0; i < size; i++) {
        if (enqueue(&recv_queue, telem_data->raw_data[i])) {
            LOG_ERROR("Receive queue full!");
            error = -1;
            break;
        }
    }

    unlock();

    // Tell Telemetry that data has been accepted
    emit_Telemetry2Decrypt_DataReadyAck_emit();

    return error;
}

// Triggered when Telemetry signals Decrypt a DataReady Event
static void consume_Telemetry2Decrypt_DataReadyEvent_callback(void *in_arg UNUSED) {
    if (read_from_telemetry()) {
        LOG_ERROR("Error reading from telemetry");
    }
    
    if (consume_Telemetry2Decrypt_DataReadyEvent_reg_callback(&consume_Telemetry2Decrypt_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyEvent callback");
    }
}

// Decrypt sends decrypted FC data to UART
// when UART gives Decrypt an ACK
static void consume_Decrypt2UART_DataReadyAck_callback(void *in_arg UNUSED) {
    if (send_to_uart()) {
        LOG_ERROR("Error sending to uart");
    }

    if (consume_Decrypt2UART_DataReadyAck_reg_callback(&consume_Decrypt2UART_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyAck callback");
    }
}

void consume_Telemetry2Decrypt_DataReadyEvent__init(void) {
    if (consume_Telemetry2Decrypt_DataReadyEvent_reg_callback(&consume_Telemetry2Decrypt_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyEvent callback");
    }
}

void consume_Decrypt2UART_DataReadyAck__init(void) {
    if (consume_Decrypt2UART_DataReadyAck_reg_callback(&consume_Decrypt2UART_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyAck callback");
    }
}

int run(void) {
    LOG_ERROR("In run");

    emit_Telemetry2Decrypt_DataReadyAck_emit();

    while (1) {
        Decrypt_Telem_Data_to_FC_Data();
    }

    return 0;
}
