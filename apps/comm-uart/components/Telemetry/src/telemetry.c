#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/chardev.h>
#include <platsupport/io.h>
#include <utils/util.h>

#include <stdint.h>
#include <string.h>

#include <top_types.h>

#define lock() \
    do { \
        if (telemetry_lock()) { \
            printf("[%s] failed to lock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

#define unlock() \
    do { \
        if (telemetry_unlock()) { \
            printf("[%s] failed to unlock: %d\n", get_instance_name(), __LINE__); \
        } \
    } while (0)

static ps_io_ops_t io_ops;
static ps_chardevice_t serial_device;
static ps_chardevice_t *serial = NULL;

// From telemetry to decrypt
static queue_t recv_queue;

// From telemetry to outside world
static queue_t send_queue;

void pre_init() {
    LOG_ERROR("Starting Telemetry");
    LOG_ERROR("In pre_init");
    int error;
    error = camkes_io_ops(&io_ops);
    ZF_LOGF_IF(error, "Failed to initialise IO ops");

    serial = ps_cdev_init(BCM2xxx_UART3, &io_ops, &serial_device);
    ZF_LOGF_IF(!serial, "Failed to initialise char device");

    queue_init(&recv_queue);
    queue_init(&send_queue);
    LOG_ERROR("Out pre_init");
}

static int telemetry_rx_poll() {
    int c = EOF;
    c = ps_cdev_getchar(serial);
    if (c == EOF) {
        return -1;
    }
    // Poll could happen when dequeueing
    lock();
    if (enqueue(recv_queue, c)) {
        LOG_ERROR("Receive queue full!");
    }
    unlock();
    return 0;
}

static int telemetry_tx_poll() {
    lock();

    int size = send_queue.size;
    char c;
    for (int i=0; i < size; i++) {
        if (!dequeue(&send_queue, &c)) {
            ps_cdev_putchar(serial, c);
        }
    }

    unlock();
}

static void send_to_decrypt() {
    // Protect recv_queue
    lock();

    int size;
    if (recv_queue.size > sizeof(Telem_Data->raw_data)) {
        size = sizeof(Telem_Data->raw_data);
    }

    Telem_Data *telem_data = (Telem_Data*) send_Telem_Data_Telemetry2Decrypt;
    uint8_t tmp;
    for (int i=0; i<size; i++) {
        dequeue(&recv_queue, &tmp);
        telem_data->raw_data[i] = tmp;
        send_Telem_Data_Telemetry2Decrypt_release();
    }
    telem_data->len = size;
    unlock();

    LOG_ERROR("To decrypt");
    // Telemetry => Decrypt
    emit_Telemetry2Decrypt_DataReadyEvent_emit();
}

// Telemetry sends data to Decrypt
// when Decrypt gives Telemetry an ACK
static void consume_Telemetry2Decrypt_DataReadyAck_callback(void *in_arg UNUSED) {
    send_to_decrypt();

    if (consume_Telemetry2Decrypt_DataReadyAck_reg_callback(&consume_Telemetry2Decrypt_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyAck callback");
    }
}

void consume_Telemetry2Decrypt_DataReadyAck__init() {
    if (consume_Telemetry2Decrypt_DataReadyAck_reg_callback(&consume_Telemetry2Decrypt_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyAck callback");
    }
}

// Telemetry receives data from Encrypt
// when Encrypt gives Telemetry a DataReady Event
static void consume_Encrypt2Telemetry_DataReadyEvent_callback(void *in_arg UNUSED) {
    recv_Telem_Data_Encrypt2Telemetry_acquire();
    Telem_Data *telem_data = (Telem_Data *) recv_Telem_Data_Encrypt2Telemetry;
    
    lock();

    if (telem_data->len + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Send queue not enough!");
    }

    for (int i=0; i < telem_data->len; i++) {
        if (enqueue(&send_queue, telem_data->raw_data[i])) {
            break;
        }
    }

    unlock();

    // Tell Encrypt that data has been accepted
    emit_Encrypt2Telemetry_DataReadyAck_emit();

    if (consume_Encrypt2Telemetry_DataReadyEvent_reg_callback(&consume_Encrypt2Telemetry_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Encrypt2Telemetry_DataReadyEvent callback");
    }
}

void consume_Encrypt2Telemetry_DataReadyEvent__init() {
    if (consume_Encrypt2Telemetry_DataReadyEvent_reg_callback(&consume_Encrypt2Telemetry_DataReadyEvent_callback, NULL)) {
        ZF_LOGF("Failed to register Encrypt2Telemetry_DataReadyEvent callback");
    }
}

int run() {
    LOG_ERROR("In run");

    while (1) {
        telemetry_rx_poll();
        telemetry_tx_poll();
    }

    return 0;
}
