#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/spi.h>
#include <platsupport/plat/spi.h>
#include <utils/util.h>
#include <stdint.h>
#include <string.h>

static ps_io_ops_t io_ops;
static spi_bus_t *spi_bus = NULL;

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
    uint8_t rxdata[txcnt];
    const uint32_t rxcnt = sizeof(rxdata);
    spi_xfer(spi_bus, txdata, txcnt, rxdata, rxcnt, NULL, NULL);
}

int run() {
    LOG_ERROR("In run");
    while (1) {

    }

    return 0;
}
