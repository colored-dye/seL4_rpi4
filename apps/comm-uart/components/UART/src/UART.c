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

#define lock() \
    do { \
        if (uart_lock()) { \
            printf("[%s] failed to lock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

#define unlock() \
    do { \
        if (uart_unlock()) { \
            printf("[%s] failed to unlock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

static ps_io_ops_t io_ops;
static ps_chardevice_t serial_device;
static ps_chardevice_t *serial = NULL;

// From Decrypt to UART
static queue_t recv_queue;

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

    LOG_ERROR("Out pre_init");
}

// UART receives data from Decrypt
// when Decrypt gives UART a DataReady Event
static void consume_Decrypt2UART_DataReadyEvent_callback(void *in_arg UNUSED) {


    if (consume_Decrypt2UART_DataReadyEvent_reg_callback(&consume_Decrypt2UART_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Decrypt2UART_DataReadyEvent callback");
    }
}

static int send_to_uart(void) {

    return 0;
}

// UART sends data to Encrypt
// when Encrypt gives UART an ACK
static void consume_UART2Encrypt_DataReadyAck_callback(void *in_arg UNUSED) {
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

static int telemetry_rx_poll(void) {
    int c = EOF;
    c = ps_cdev_getchar(serial);
    if (c == EOF) {
        return -1;
    }

    lock();
    if (enqueue(&recv_queue, c)) {
        LOG_ERROR("Receive queue full!");
    }
    unlock();

    return 0;
}

static int telemetry_tx_poll(void) {
    lock();

    int size = send_queue.size;
    uint8_t c;
    for (uint32_t i = 0; i < size; i++) {
        if (!dequeue(&send_queue, &c)) {
            ps_cdev_putchar(serial, c);
        }
    }

    unlock();
    return 0;
}

int run(void) {
    LOG_ERROR("In run");
    while (1) {

    }
    
    return 0;
}
