#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/spi.h>
#include <platsupport/plat/spi.h>
#include <utils/util.h>

static ps_io_ops_t io_ops;
static spi_bus_t *spi_bus;

void pre_init() {
    LOG_ERROR("In pre_init");
    if (!spi_init(SPI0, &io_ops, &spi_bus)) {
        LOG_ERROR("spi_bus: %p", spi_bus);
    }
}

int run() {
    LOG_ERROR("In run");
    while (1) {

    }

    return 0;
}
