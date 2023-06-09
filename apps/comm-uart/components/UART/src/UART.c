#include "camkes-component-uart.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/chardev.h>
#include <platsupport/io.h>
#include <utils/util.h>

#include <top_types.h>

#define send_wait() \
    do { \
        if (uart_send_wait()) { \
            LOG_ERROR("[%s] failed to lock\n", get_instance_name()); \
        } \
    } while (0)

#define send_post() \
    do { \
        if (uart_send_post()) { \
            LOG_ERROR("[%s] failed to post\n", get_instance_name()); \
        } \
    } while (0)

#define recv_wait() \
    do { \
        if (uart_recv_wait()) { \
            LOG_ERROR("[%s] failed to lock\n", get_instance_name()); \
        } \
    } while (0)

#define recv_post() \
    do { \
        if (uart_recv_post()) { \
            LOG_ERROR("[%s] failed to post\n", get_instance_name()); \
        } \
    } while (0)

static ps_io_ops_t io_ops;
static ps_chardevice_t serial_device;
static ps_chardevice_t *serial = NULL;

// From UART to Encrypt,
// From outside world to UART
static queue_t recv_queue;

// From Decrypt to UART,
// From UART to outside world
static queue_t send_queue;


void pre_init() {
    LOG_ERROR("Starting UART");
    LOG_ERROR("In pre_init");

    int error;
    error = camkes_io_ops(&io_ops);
    ZF_LOGF_IF(error, "Failed to initialise IO ops");

    serial = ps_cdev_init(BCM2xxx_UART5, &io_ops, &serial_device);
    ZF_LOGF_IF(!serial, "Failed to initialise char device");

    queue_init(&recv_queue);
    queue_init(&send_queue);

    FC_Data *fc_data = (FC_Data *) send_FC_Data_UART2Encrypt;
    fc_data->len = 0;
    send_FC_Data_UART2Encrypt_release();
    memset(fc_data->raw_data, -1, sizeof(fc_data->raw_data));

    recv_post();
    send_post();

    LOG_ERROR("Out pre_init");
}

// Send FC data to Encrypt
// from recv_queue
static int send_to_encrypt(void) {
    int error = 0;

    uint32_t queue_size;
    uint32_t data_size;

    // Wait until recv_queue is not empty
    while (1) {
        if (recv_queue.size > 0) {
            break;
        }
    }

    recv_wait();

    data_size = recv_queue.size;
    if (data_size > sizeof(FC_Data_raw)) {
        data_size = sizeof(FC_Data_raw);
    }
    FC_Data *fc_data = (FC_Data *) send_FC_Data_UART2Encrypt;
    uint8_t tmp;
    for (uint32_t i = 0; i < data_size; i++) {
        if (!dequeue(&recv_queue, &tmp)) {
            fc_data->raw_data[i] = tmp;
            send_FC_Data_UART2Encrypt_release();
        } else {
            LOG_ERROR("Should not get here");
            data_size = 0;
            error = -1;
            break;
        }
    }
    fc_data->len = data_size;

    recv_post();

    // LOG_ERROR("To encrypt");
    // Tell Encrypt that data is ready
    emit_UART2Encrypt_DataReadyEvent_emit();

    return error;
}

// Read decrypted FC data from Decrypt
// and push to send_queue
static int read_from_decrypt(void) {
    int error = 0;

    FC_Data *fc_data = (FC_Data *) recv_FC_Data_Decrypt2UART;

    send_wait();

    uint32_t data_len = fc_data->len;
    if (data_len + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Receivce queue not enough!");
    }

    for (uint32_t i = 0; i < data_len; i++) {
        recv_FC_Data_Decrypt2UART_acquire();
        if (enqueue(&send_queue, fc_data->raw_data[i])) {
            LOG_ERROR("Receive queue full!");
            error = -1;
            break;
        }
    }

    send_post();

    // Tell Decrypt that data has been accepted
    emit_Decrypt2UART_DataReadyAck_emit();

    return error;
}

// UART receives data from Decrypt
// when Decrypt gives UART a DataReady Event
static void consume_Decrypt2UART_DataReadyEvent_callback(void *in_arg UNUSED) {
    if (read_from_decrypt()) {
        LOG_ERROR("Error reading from decrypt");
    }

    if (consume_Decrypt2UART_DataReadyEvent_reg_callback(&consume_Decrypt2UART_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyEvent callback");
    }
}

// UART sends data to Encrypt
// when Encrypt gives UART an ACK
static void consume_UART2Encrypt_DataReadyAck_callback(void *in_arg UNUSED) {
    if (send_to_encrypt()) {
        LOG_ERROR("Error sending to encrypt");
    }

    if (consume_UART2Encrypt_DataReadyAck_reg_callback(&consume_UART2Encrypt_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register UART2Encrypt_DataReadyAck callback");
    }
}

void consume_Decrypt2UART_DataReadyEvent__init(void) {
    if (consume_Decrypt2UART_DataReadyEvent_reg_callback(&consume_Decrypt2UART_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyEvent callback");
    }
}

void consume_UART2Encrypt_DataReadyAck__init(void) {
    if (consume_UART2Encrypt_DataReadyAck_reg_callback(&consume_UART2Encrypt_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register UART2Encrypt_DataReadyAck callback");
    }
}

// Read from RX and push to recv_queue
static int uart_rx_poll(void) {
    // int c = EOF;
    // c = ps_cdev_getchar(serial);
    // if (c == EOF) {
    //     return -1;
    // }

    // recv_wait();
    // if (enqueue(&recv_queue, c)) {
    //     LOG_ERROR("Receive queue full!");
    // }
    // recv_post();

    uint8_t buf[32];
    uint32_t len = ps_cdev_read(serial, buf, sizeof(buf), NULL, NULL);
    recv_wait();
    for (uint32_t i = 0; i < len; i++) {
        if (enqueue(&recv_queue, buf[i])) {
            LOG_ERROR("Receive queue full!");
            break;
        }
    }
    recv_post();

    return 0;
}

// Write to TX from send_queue
static int uart_tx_poll(void) {
    int error = 0;
    send_wait();

    // if (!queue_empty(&send_queue)) {
    //     print_queue(&send_queue);
    //     // print_queue_serial(&send_queue);
    // }

    uint32_t size = send_queue.size;
    uint8_t c;
    for (uint32_t i = 0; i < size; i++) {
        if (!dequeue(&send_queue, &c)) {
            ps_cdev_putchar(serial, c);
        } else {
            error = -1;
            break;
        }
    }

    send_post();
    return error;
}

int run(void) {
    LOG_ERROR("In run");

    emit_Decrypt2UART_DataReadyAck_emit();

    while (1) {
        uart_rx_poll();
        uart_tx_poll();
    }
    
    return 0;
}
