#include <camkes.h>
#include <platsupport/spi.h>
#include <platsupport/gpio.h>
#include <utils/util.h>

void spi_irq_callback(void *in_arg) {
    LOG_ERROR("In spi_irq_callback");
}

void spi__init() {
    LOG_ERROR("In spi__init");
    spi_int_reg_callback(&spi_irq_callback, NULL);

    // printf("spi reg: %p\n", spi_reg);
}

int spi_transfer(int id, unsigned int wcount, unsigned int rcount) {
    return 0;
}

int run() {
    LOG_ERROR("In run");

    while (1) {

    }

    return 0;
}
