#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/chardev.h>
#include <stdint.h>
#include <utils/util.h>

#include "my_type.h"

static ps_io_ops_t io_ops;
static ps_chardevice_t serial_device;
static ps_chardevice_t *serial = NULL;

void pre_init() {
  ZF_LOGE("In pre_init");

  int error;
  ring_buffer_t *rb;

  error = camkes_io_ops(&io_ops);
  ZF_LOGF_IF(error, "Failed to initialise IO ops");

  serial = ps_cdev_init(TELEMETRY_PORT_NUMBER, &io_ops, &serial_device);
  if (serial == NULL) {
    ZF_LOGF("Failed to initialise char device");
  }

  rb = (ring_buffer_t *)ring_buffer;
  rb->head = 0;
  ring_buffer_release();
  rb->tail = 0;
  ring_buffer_release();

  LOG_ERROR("Out pre_init");
}

void serial__init() {}

void serial_send(const uint8_t *buf, uint16_t len) {
  if (ps_cdev_write(serial, (void *)buf, len, NULL, NULL) != len) {
    LOG_ERROR("Write not completed");
  }
}

int run(void) {
  ZF_LOGE("In run");

  int c;
  ring_buffer_t *rb = (ring_buffer_t *)ring_buffer;
  uint32_t tail;

  while (1) {
    while ((c = ps_cdev_getchar(serial)) == EOF) {
    }

    tail = rb->tail;
    ring_buffer_acquire();
    rb->buffer[tail] = c;
    ring_buffer_release();
    tail = (tail + 1) % RING_BUFFER_SIZE;
    rb->tail = tail;
    ring_buffer_release();
  }

  LOG_ERROR("Out run");
  return 0;
}
