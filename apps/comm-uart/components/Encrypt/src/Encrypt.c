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
            LOG_ERROR("[%s] failed to lock\n", get_instance_name()); \
        } \
    } while (0)

#define unlock() \
    do { \
        if (encrypt_unlock()) { \
            LOG_ERROR("[%s] failed to unlock\n", get_instance_name()); \
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

// Simulate encryption.
// No encryption now, so just give FC_Data to Telem_Data.
// From recv_queue to send_queue.
// TODO: Implement actual encryption
static void Encrypt_FC_Data_to_Telem_Data() {
    // Protect both recv_queue and send_queue
    lock();

    if (recv_queue.size + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Send queue not enough!");
    }

    for (uint32_t i=0; i < recv_queue.size; i++) {
        uint8_t tmp;
        if (dequeue(&recv_queue, &tmp)) {
            break;
        }
        // putchar(tmp);
        if (enqueue(&send_queue, tmp)) {
            break;
        }
    }

    unlock();
}

// Send encrypted Telem data to Telemetry
// through shared memory
// from send_queue
static int send_to_telemetry(void) {
    int error = 0;

    uint32_t queue_size;
    uint32_t data_size;

    // Wait until send_queue is not empty,
    // because xxxAck_callback is invoked
    // once for each ACK
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

    data_size = send_queue.size;
    if (data_size > sizeof(Telem_Data_raw)) {
        data_size = sizeof(Telem_Data_raw);
    }

    Telem_Data *telem_data = (Telem_Data *) send_Telem_Data_Encrypt2Telemetry;
    uint8_t tmp;
    for (uint32_t i = 0; i < data_size; i++) {
        if (!dequeue(&send_queue, &tmp)) {
            telem_data->raw_data[i] = tmp;
            send_Telem_Data_Encrypt2Telemetry_release();
        } else {
            LOG_ERROR("Should not get here");
            error = -1;
            data_size = 0;
            break;
        }
    }
    telem_data->len = data_size;

    unlock();

    LOG_ERROR("To telemetry");
    // Encrypt => Telemetry
    emit_Encrypt2Telemetry_DataReadyEvent_emit();

    return error;
}

// Read FC data from UART
// and push to recv_queue
static int read_from_uart(void) {
    int error = 0;

    // Protect recv_queue
    lock();

    recv_FC_Data_UART2Encrypt_acquire();
    FC_Data *fc_data = (FC_Data *) recv_FC_Data_UART2Encrypt;
    uint32_t size = fc_data->len;

    for (uint32_t i = 0; i < size; i++) {
        if (enqueue(&recv_queue, fc_data->raw_data[i])) {
            LOG_ERROR("Receive queue full!");
            error = -1;
            break;
        }
    }

    unlock();

    // Tell UART that data has been accepted
    emit_UART2Encrypt_DataReadyAck_emit();

    return error;
}

// Encrypt sends encrypted Telem data to Telemetry
// when Telemetry gives Encrypt an ACK
static void consume_Encrypt2Telemetry_DataReadyAck_callback(void *in_arg) {
    if (send_to_telemetry()) {
        LOG_ERROR("Error sending to telemetry");
    }

    if (consume_Encrypt2Telemetry_DataReadyAck_reg_callback(&consume_Encrypt2Telemetry_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Encrypt2Telemetry_DataReadyAck callback");
    }
}

// Encrypt reads FC data from UART
// when UART gives Encrypt a DataReady Event
static void consume_UART2Encrypt_DataReadyEvent_callback(void *in_arg) {
    if (read_from_uart()) {
        LOG_ERROR("Error reading from uart");
    }
    
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

    emit_UART2Encrypt_DataReadyAck_emit();

    while (1) {
        Encrypt_FC_Data_to_Telem_Data();
    }

    return 0;
}
