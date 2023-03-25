#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/spi.h>
#include <platsupport/plat/spi.h>
#include <utils/util.h>
#include <stdint.h>
#include <string.h>

static ps_io_ops_t io_ops;
static spi_bus_t *spi_bus;

typedef volatile struct spi_regs {
    uint32_t cs;    // master control and status
    uint32_t fifo;  // TX and RX FIFOs. Data, RW.
    uint32_t clk;   // clock divider
    uint32_t dlen;  // data length
    uint32_t ltoh;  // LoSSI mode TOH
    uint32_t dc;    // DMA DREQ controls
} spi_regs_t;

struct spi_bus {
    spi_regs_t* regs;

    clk_t *clk;

    const uint8_t *txbuf;
    uint8_t *rxbuf;
    size_t txsize, rxsize;
    spi_chipselect_fn cs;
    spi_callback_fn cb;
    void* token;
};

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

    // uint8_t txdata[] = "Wow, hello there! I am foo";
    // const uint32_t txcnt = strlen(txdata);
    // spi_xfer(spi_bus, txdata, txcnt, NULL, 0, NULL, NULL);

    // Register irq handler
    ps_irq_t irq_info = {
        .type = PS_INTERRUPT,
        .irq = {
            .number = 150,
        }
    };
    irq_id_t serial_irq_id = ps_irq_register(&io_ops.irq_ops, irq_info, spi_irq_handle, NULL);
    ZF_LOGF_IF(serial_irq_id < 0, "Failed to register irq");

    spi_bus->regs->cs |= BIT(7);
}

int run() {
    LOG_ERROR("In run");
    while (1) {

    }

    return 0;
}
