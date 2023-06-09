#include "platsupport/gpio.h"
#include "platsupport/io.h"
#include "utils/util.h"
#include <platsupport/spi.h>

#include <platsupport/plat/spi.h>
#include <platsupport/plat/gpio.h>

#include <stdint.h>

#define SPI0_PADDR 0xfe204000
#define SPI0_IRQ 150

// GPIO -> SPI
typedef struct spi_gpio_defn {
    int spi_id;
    int spi_ce1;
    int spi_ce0;
    int spi_miso;
    int spi_mosi;
    int spi_sclk;
    int alt_function;
} spi_gpio_defn_t;

#define SPI_GPIO_DEFN(id, ce1, ce0, miso, mosi, sclk, alt) {   \
    .spi_id         = id,   \
    .spi_ce1        = ce1,  \
    .spi_ce0        = ce0,  \
    .spi_miso       = miso, \
    .spi_mosi       = mosi, \
    .spi_sclk       = sclk, \
    .alt_function   = alt   \
}

#if defined(CONFIG_PLAT_BCM2711)
static const spi_gpio_defn_t gpio_defs[NSPI] = {
    SPI_GPIO_DEFN(SPI0, 7, 8, 9, 10, 11, BCM2711_GPIO_FSEL_ALT0)
};
#endif

typedef volatile struct spi_regs {
    uint32_t cs;    // master control and status
    uint32_t fifo;  // TX and RX FIFOs. Data, RW.
    uint32_t clk;   // clock divider
    uint32_t dlen;  // data length
    uint32_t ltoh;  // LoSSI mode TOH
    uint32_t dc;    // DMA DREQ controls
} spi_regs_t;

// SPI Registers

/*************************
* CS Register
*************************/
#define SPI_CS_LEN_LONG BIT(25)
#define SPI_CS_DMA_LEN  BIT(24)
#define SPI_CS_CSPOL2   BIT(23)
#define SPI_CS_CSPOL1   BIT(22)
#define SPI_CS_CSPOL0   BIT(21) // Chip select 0 polarity
#define SPI_CS_RXF      BIT(20) // RX FIFO full
#define SPI_CS_RXR      BIT(19) // RX FIFO needs reading
#define SPI_CS_TXD      BIT(18) // TX FIFO can accept data
#define SPI_CS_RXD      BIT(17) // RX FIFO contains data
#define SPI_CS_DONE     BIT(16) // Transfer done
#define SPI_CS_TE_EN    BIT(15) // UNUSED
#define SPI_CS_LMONO    BIT(14) // UNUSED
#define SPI_CS_LEN      BIT(13) // LoSSI enable
#define SPI_CS_REN      BIT(12) // Read enable
#define SPI_CS_ADCS     BIT(11) // Automatically de-assert chip select
#define SPI_CS_INTR     BIT(10) // Interrupt on RXR
#define SPI_CS_INTD     BIT(9)  // Interrupt on DONE
#define SPI_CS_DMAEN    BIT(8)  // DMA enable
#define SPI_CS_TA       BIT(7)  // Transfer active
#define SPI_CS_CSPOL    BIT(6)  // Chip select polarity
#define SPI_CS_CLEAR_RX BIT(5)
#define SPI_CS_CLEAR_TX BIT(4)
#define SPI_CS_CPOL     BIT(3)  // Clock polarity
#define SPI_CS_CPHA     BIT(2)  // Clock phase
#define SPI_CS_CS       0x3     // Chip select

/*************************
* CLK Register
*************************/
#define SPI_CLK_CDIV    0xff    // Clock divider. SCLK = Core Clock / CDIV

/*************************
* DLEN Register
*************************/
#define SPI_DLEN_LEN    0xff    // Data length. Only valid for DMA mode.

/*************************
* LTOH Register
*************************/
#define SPI_LTOH_TOH    0xf     // Output hold delay

#define SPI_TX_FIFO_LEN 64
#define SPI_RX_FIFO_LEN 48

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

static spi_bus_t _spi[NSPI] = {
    [SPI0] = {
        .regs = NULL,
    },
};
static spi_bus_t *g_spi_bus = NULL;

int spi_gpio_configure(enum spi_id id, const ps_io_ops_t *io_ops) {
    gpio_sys_t gpio_sys;
    gpio_t gpio;

    spi_gpio_defn_t pindef = { -1, -1, -1, -1, -1, -1, -1 };
    for (int i = 0; i < NSPI; i++) {
        if (gpio_defs[i].spi_id == id) {
            pindef = gpio_defs[i];
            break;
        }
    }

    if (pindef.spi_id < 0) {
        ZF_LOGE("No valid pin configuration found for spi: %i", id);
        return -1;
    }

    // LOG_ERROR("Before gpio_sys_init");
    int error = gpio_sys_init((ps_io_ops_t*) io_ops, &gpio_sys);
    if (error) {
        ZF_LOGE("GPIO sys init failed: %i", error);
        return -1;
    }
    // LOG_ERROR("After gpio_sys_init");

#if defined(CONFIG_PLAT_BCM2711)
    gpio_sys.init(&gpio_sys, pindef.spi_ce1, 0, &gpio);
    bcm2711_gpio_fsel(&gpio, pindef.alt_function);

    gpio_sys.init(&gpio_sys, pindef.spi_ce0, 0, &gpio);
    bcm2711_gpio_fsel(&gpio, pindef.alt_function);

    gpio_sys.init(&gpio_sys, pindef.spi_miso, 0, &gpio);
    bcm2711_gpio_fsel(&gpio, pindef.alt_function);

    gpio_sys.init(&gpio_sys, pindef.spi_mosi, 0, &gpio);
    bcm2711_gpio_fsel(&gpio, pindef.alt_function);

    gpio_sys.init(&gpio_sys, pindef.spi_sclk, 0, &gpio);
    bcm2711_gpio_fsel(&gpio, pindef.alt_function);
#endif

    return 0;
}

static void spi_irq_handle(void *data, ps_irq_acknowledge_fn_t acknowledge_fn, void *ack_data) {
    int error;

    spi_handle_irq(g_spi_bus);

    error = acknowledge_fn(ack_data);
    ZF_LOGF_IF(error, "Failed to acknowledge IRQ");
}

int spi_init(enum spi_id id, ps_io_ops_t *io_ops, spi_bus_t **ret_spi_bus) {
    ZF_LOGE("In spi_init");
    spi_bus_t *spi_bus = &_spi[id];
    g_spi_bus = spi_bus;

    spi_gpio_configure(id, io_ops);

    void *vaddr = ps_io_map(&io_ops->io_mapper, SPI0_PADDR, BIT(12), 0, PS_MEM_NORMAL);

    ZF_LOGE("vaddr: %p", vaddr);

    spi_bus->regs = (spi_regs_t*)vaddr;

    // CS Register
    spi_bus->regs->cs = (0 & SPI_CS_CS);    // Chip select 0
    spi_bus->regs->cs |= SPI_CS_CPOL;
    spi_bus->regs->cs |= SPI_CS_CPHA;
    spi_bus->regs->cs |= SPI_CS_CSPOL;
    spi_bus->regs->cs |= SPI_CS_CSPOL0;

    // Set interrupt
    // spi_bus->regs->cs |= SPI_CS_INTD;   // Interrupt on DONE
    // spi_bus->regs->cs |= SPI_CS_INTR;   // Interrupt on RXR
    // ps_irq_t irq_info = {
    //     .type = PS_INTERRUPT,
    //     .irq = {
    //         .number = 12,
    //     }
    // };
    // irq_id_t serial_irq_id = ps_irq_register(&io_ops->irq_ops, irq_info, spi_irq_handle, NULL);
    // ZF_LOGF_IF(serial_irq_id < 0, "Failed to register irq");

    // CLK Register
    spi_bus->regs->clk = (128 & SPI_CLK_CDIV); // 250MHz / CDIV = SPI speed rate

    print_spi_regs(spi_bus->regs);

    spi_bus->cs = NULL;
    spi_bus->cb = NULL;
    spi_bus->token = NULL;

    *ret_spi_bus = spi_bus;

    spi_bus->regs->cs |= SPI_CS_TA;
    return 0;
}

long spi_set_speed(spi_bus_t *spi_bus, long bps) {
    return 0;
}

void spi_handle_irq(spi_bus_t *dev) {
    LOG_ERROR("In spi_handle_irq");

    print_spi_regs(dev->regs);

    // if (dev->regs->cs & SPI_CS_DONE) {
    //     for (int i=0; i<dev->txsize; i++) {
    //         dev->regs->fifo = dev->txbuf[i];
    //     }
    // }

    // dev->regs->cs &= ~SPI_CS_TA;

    // while (dev->regs->cs & SPI_CS_RXD) {
    //     printf("%c ", dev->regs->fifo);
    // }
    // putchar('\n');

    // print_spi_regs(dev->regs);

    // if (dev->regs->cs & SPI_CS_RXR) {
    //     LOG_ERROR("RXR is set");
    // }

    // print_spi_regs(dev->regs);

    LOG_ERROR("Out spi_handle_irq");
}

void spi_prepare_transfer(spi_bus_t *spi_bus, const spi_slave_config_t *cfg) {
    /* nothing to do */
}

static void spi_transfer_poll(spi_bus_t *spi_bus) {
    LOG_ERROR("In spi_transfer_poll");

    // Use Poll
    size_t size = spi_bus->txsize;

    // Set TA = 1
    spi_bus->regs->cs |= SPI_CS_TA;

    LOG_ERROR("Send data");
    int wcnt = 0;
    int rcnt = 0;
    while (spi_bus->rxsize > 0) {
        uint32_t cs = spi_bus->regs->cs;
        if (cs & SPI_CS_TXD) {
            if (spi_bus->txsize > 0) {
                LOG_ERROR("Write %d", ++wcnt);
                spi_bus->regs->fifo = *(spi_bus->txbuf);
                spi_bus->txbuf++;
                spi_bus->txsize--;
            }
        }

        if (cs & SPI_CS_RXD) {
            LOG_ERROR("Read %d", ++rcnt);
            *(spi_bus->rxbuf) = spi_bus->regs->fifo;
            spi_bus->rxbuf++;
            spi_bus->rxsize--;
        }
    }

    // Poll DONE until it goes to 1
    while (!(spi_bus->regs->cs & SPI_CS_DONE)) {

    }

    // Set TA = 0
    spi_bus->regs->cs &= ~SPI_CS_TA;

    print_spi_regs(spi_bus->regs);

    LOG_ERROR("Out spi_transfer_poll");
}

static void spi_transfer(spi_bus_t *spi_bus) {
    LOG_ERROR("In spi_transfer");

    spi_transfer_poll(spi_bus);

    LOG_ERROR("Out spi_transfer");
}

static void spi_read(spi_bus_t *spi_bus) {
    LOG_ERROR("In spi_read");

    // Set TA = 1
    spi_bus->regs->cs |= SPI_CS_TA;

    int i = 0;
    while (i < spi_bus->rxsize && spi_bus->regs->cs & SPI_CS_RXD) {
        spi_bus->rxbuf[i] = spi_bus->regs->fifo;
        printf("%02X ", spi_bus->rxbuf[i]);
    }
    putchar('\n');

    print_spi_regs(spi_bus->regs);

    spi_bus->regs->cs &= ~SPI_CS_TA;

    LOG_ERROR("Out spi_read");
}

int spi_xfer(
    spi_bus_t *spi_bus,
    const void *txdata,
    size_t txcnt,
    void *rxdata,
    size_t rxcnt,
    spi_callback_fn cb,
    void *token
    )
{
    LOG_ERROR("In spi_xfer");

    spi_bus->txbuf = (const uint8_t*) txdata;
    spi_bus->txsize = txcnt;
    spi_bus->rxbuf = (uint8_t*) rxdata;
    spi_bus->rxsize = rxcnt;
    spi_bus->cb = cb;
    spi_bus->token = token;

    spi_transfer(spi_bus);
    // spi_read(spi_bus);

    LOG_ERROR("Out spi_xfer");

    return 0;
}
