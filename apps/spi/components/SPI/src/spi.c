#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/spi.h>
#include <platsupport/plat/spi.h>
#include <utils/util.h>
#include <stdint.h>
#include <string.h>

static ps_io_ops_t io_ops;
static spi_bus_t *spi_bus;

static void spi_irq_handle(void *data, ps_irq_acknowledge_fn_t acknowledge_fn, void *ack_data) {
    int error;

    spi_handle_irq(spi_bus);

    error = acknowledge_fn(ack_data);
    ZF_LOGF_IF(error, "Failed to acknowledge IRQ");
}

void pre_init() {
    LOG_ERROR("In pre_init");
    int error;
    error = camkes_io_ops(&io_ops);
    ZF_LOGF_IF(error, "Failed to initialise IO ops");

    if (!spi_init(SPI0, &io_ops, &spi_bus)) {
        LOG_ERROR("spi_bus: %p", spi_bus);
    }

    uint8_t txdata[] = "Wow, hello there! I am foo";
    const uint32_t txcnt = sizeof(txdata) - 1;
    spi_xfer(spi_bus, txdata, txcnt, NULL, 0, NULL, NULL);
}

int run() {
    LOG_ERROR("In run");
    while (1) {

    }

    return 0;
}
