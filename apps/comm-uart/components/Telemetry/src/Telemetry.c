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

// From Telemetry to Decrypt
// From outside world to Telemetry
static queue_t recv_queue;

// From Telemetry to outside world
// From Encrypt to Telemetry
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
    if (enqueue(&recv_queue, c)) {
        LOG_ERROR("Receive queue full!");
    }
    unlock();

    LOG_ERROR("RX: %c", c);

    return 0;
}

static int telemetry_tx_poll() {
    int error = 0;
    lock();

    int size = send_queue.size;
    uint8_t c;

    if (!queue_empty(&send_queue)) {
        print_queue(&send_queue);
        // print_queue_serial(&send_queue);
    }

    for (uint32_t i=0; i < size; i++) {
        if (!dequeue(&send_queue, &c)) {
            ps_cdev_putchar(serial, c);
        } else {
            unlock();
            error = -1;
        }
    }

    unlock();

    return error;
}

// Send encrypted Telem data to Decrypt
// from recv_queue
static int send_to_decrypt(void) {
    int error = 0;

    int queue_size;
    int data_size;

    // Wait for RX to push some data into recv_queue
    while (1) {
        lock();
        queue_size = recv_queue.size;
        unlock();
        if (queue_size > 0) {
            break;
        }
    }

    lock();
    if (recv_queue.size > sizeof(Telem_Data_raw)) {
        data_size = sizeof(Telem_Data_raw);
    } else {
        data_size = recv_queue.size;
    }

    Telem_Data *telem_data = (Telem_Data*) send_Telem_Data_Telemetry2Decrypt;
    uint8_t tmp;
    for (uint32_t i=0; i < data_size; i++) {
        if (!dequeue(&recv_queue, &tmp)) {
            telem_data->raw_data[i] = tmp;
            send_Telem_Data_Telemetry2Decrypt_release();
        } else {
            LOG_ERROR("Should not get here");
            data_size = 0;
            error = -1;
        }
    }
    telem_data->len = data_size;
    unlock();

    LOG_ERROR("To decrypt");
    // Telemetry => Decrypt
    emit_Telemetry2Decrypt_DataReadyEvent_emit();

    return error;
}

// Telemetry sends data to Decrypt
// when Decrypt gives Telemetry an ACK
static void consume_Telemetry2Decrypt_DataReadyAck_callback(void *in_arg UNUSED) {
    if (send_to_decrypt()) {
        LOG_ERROR("Error sending to decrypt");
    }

    if (consume_Telemetry2Decrypt_DataReadyAck_reg_callback(&consume_Telemetry2Decrypt_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyAck callback");
    }
}

void consume_Telemetry2Decrypt_DataReadyAck__init() {
    if (consume_Telemetry2Decrypt_DataReadyAck_reg_callback(&consume_Telemetry2Decrypt_DataReadyAck_callback, NULL)) {
        ZF_LOGF("Failed to register Telemetry2Decrypt_DataReadyAck callback");
    }
}

// Read encrypted Telem data from Encrypt
// and push to send_queue
static int read_from_encrypt(void) {
    int error = 0;

    recv_Telem_Data_Encrypt2Telemetry_acquire();
    Telem_Data *telem_data = (Telem_Data *) recv_Telem_Data_Encrypt2Telemetry;
    
    lock();

    if (telem_data->len + send_queue.size > MAX_QUEUE_SIZE) {
        LOG_ERROR("Send queue not enough!");
    }

    for (uint32_t i=0; i < telem_data->len; i++) {
        if (enqueue(&send_queue, telem_data->raw_data[i])) {
            LOG_ERROR("Send queue full!");
            error = -1;
            break;
        }
    }

    unlock();

    // Tell Encrypt that data has been accepted
    emit_Encrypt2Telemetry_DataReadyAck_emit();

    return error;
}

// Telemetry receives data from Encrypt
// when Encrypt gives Telemetry a DataReady Event
static void consume_Encrypt2Telemetry_DataReadyEvent_callback(void *in_arg UNUSED) {
    if (read_from_encrypt()) {
        LOG_ERROR("Error reading from encrypt");
    }

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

    emit_Encrypt2Telemetry_DataReadyAck_emit();

    while (1) {
        telemetry_rx_poll();
        telemetry_tx_poll();
    }

    return 0;
}
