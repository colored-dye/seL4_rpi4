#include <camkes.h>
#include <platsupport/chardev.h>
#include <platsupport/serial.h>
#include <utils/util.h>
#include <stdio.h>
// #include <platsupport/mach/serial.h>

static ps_chardevice_t serial_device;

struct uart_token {
    size_t cur_bytes;
    size_t req_bytes;
    char* buf;
};
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int pl011_uart_init(const struct dev_defn *defn, const ps_io_ops_t *ops, ps_chardevice_t *dev);

int pl011_uart_getchar(ps_chardevice_t *dev);

int pl011_uart_putchar(ps_chardevice_t *dev, int c);

static void interrupt_event(void *token) {
    ps_chardevice_t *dev;
    dev = (ps_chardevice_t*)token;
    ps_cdev_handle_irq(dev, 0);
    interrupt_reg_callback(&interrupt_event, token);
}

int serial_configure(ps_chardevice_t *d, long bps, int char_size,
                     enum serial_parity parity, int stop_bits)
{
}

typedef volatile struct pl011_regs_s {
    uint32_t dr;            // 0x00: data register
    uint32_t rsrecr;        // 0x04: receive status/error clear register
    uint64_t unused0[2];    // 0x08
    uint32_t fr;            // 0x18: flag register
    uint32_t unused1;       // 0x1c
    uint32_t ilpr;          // 0x20: not in use
    uint32_t ibrd;          // 0x24: integer baud rate divisor
    uint32_t fbrd;          // 0x28: fractional baud rate divisor
    uint32_t lcrh;          // 0x2c: line control register
    uint32_t cr;            // 0x30: control register
    uint32_t ifls;          // 0x34: interrupt FIFO level select register
    uint32_t imsc;          // 0x38: interrupt mask set clear register
    uint32_t ris;           // 0x3c: raw interrupt status register
    uint32_t mis;           // 0x40: masked interrupt status register
    uint32_t icr;           // 0x44: interrupt clear register
    uint32_t dmacr;         // 0x48: DMA control register
}
pl011_regs_t;

/* LCRH register */
#define LCRH_SPS        BIT(7)   // Stick parity select
#define LCRH_WLEN_8BIT  (3 << 5) // Word length
#define LCRH_WLEN_7BIT  (2 << 5) // Word length
#define LCRH_WLEN_6BIT  (1 << 5) // Word length
#define LCRH_WLEN_5BIT  (0 << 5) // Word length
#define LCRH_FEN        BIT(4)   // Enable FIFOs
#define LCRH_STP2       BIT(3)   // Two stop bits select
#define LCRH_EPS        BIT(2)   // Even parity select
#define LCRH_PEN        BIT(1)   // Parity enable
#define LCRH_BRK        BIT(0)   // Send break

/* CR register */
#define CR_RXE          BIT(9) // Receive enable
#define CR_TXE          BIT(8) // Transmit enable
#define CR_UARTEN       BIT(0) // UART enable

/* FR register */
#define FR_TXFE         BIT(7) // Transmit FIFO empty
#define FR_RXFF         BIT(6) // Receive FIFO full
#define FR_TXFF         BIT(5) // Transmit FIFO full
#define FR_RXFE         BIT(4) // Receive FIFO empty
#define FR_BUSY         BIT(3) // UART busy

/* IMSC register */
#define IMSC_RXIM       BIT(4)
#define IMSC_TXIM       BIT(5)

/* ICR register */
#define ICR_RXIC        BIT(4)
#define ICR_TXIC        BIT(5)


static inline pl011_regs_t *pl011_uart_get_priv(ps_chardevice_t *dev)
{
    return (pl011_regs_t *)(dev->vaddr);
}

static inline void pl011_uart_enable(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    r->cr |= CR_UARTEN;
}

static inline void pl011_uart_disable(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    r->cr &= ~CR_UARTEN;
}

static inline void pl011_uart_enable_fifo(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    r->lcrh |= LCRH_FEN;
}

static inline void pl011_uart_disable_fifo(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    r->lcrh &= ~LCRH_FEN;
}

static inline void pl011_uart_enable_rx_irq(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    r->imsc |= IMSC_RXIM;
}

static inline void pl011_uart_disable_rx_irq(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    r->imsc &= ~IMSC_RXIM;
}

static inline void pl011_uart_wait_busy(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    while (r->fr & FR_BUSY);
}

static void pl011_uart_handle_irq(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);

    // Clear (all) interrupts
    r->icr = 0x7ff;
}

static int pl011_uart_cr_configure(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    uint32_t val = r->cr;

    val |= CR_TXE; // Transmit enable
    val |= CR_RXE; // Teceive enable

    r->cr = val;

    return 0;
}

static int pl011_uart_lcrh_configure(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);
    uint32_t val = r->lcrh;

    val |= LCRH_WLEN_8BIT; // Character size is 8bit
    val &= ~LCRH_STP2;     // Only one stop bit
    val &= ~LCRH_PEN;      // No parity

    r->lcrh = val;

    return 0;
}

/*
 * Set UART baud rate divisor value
 *
 * More information: https://developer.arm.com/documentation/ddi0183/g/programmers-model/register-descriptions/fractional-baud-rate-register--uartfbrd
 */
static int pl011_uart_baudrate_div_configure(ps_chardevice_t *dev)
{
    // Base UART clock according to https://github.com/raspberrypi/firmware/issues/951
    const uint32_t freq_uart_clk = 48000000;
    const uint32_t baud_rate = 115200;
    // const uint32_t baud_rate = 57600;

    double baud_div = (double) freq_uart_clk / (double)(16.0 * baud_rate);
    double frac_div = baud_div - (uint32_t) baud_div;
    pl011_regs_t *r = pl011_uart_get_priv(dev);

    // Set IBRD register
    uint32_t val = r->ibrd;
    val |= (uint32_t) baud_div;
    r->ibrd = val;

    // Set FBRD register
    val = r->fbrd;
    val |= (uint32_t)(frac_div * 64.0 + 0.5);
    r->fbrd = val;

    return 0;
}

/*
 * Configure UART registers
 *
 * This configuration process is based on the description in the BCM2711 TRM
 * section 11.5 Register View, more precisely on the information given for the
 * CR register. Furthermore, the LCRH and the baudrate divider registers (IBRD,
 * FBRD) are configured. For all of these registers the UART must be disabled.
 */
static int pl011_uart_configure(ps_chardevice_t *dev)
{
    pl011_regs_t *r = pl011_uart_get_priv(dev);

    // Disable UART
    pl011_uart_disable(dev);

    // Disable RX/all interrupts
    //pl011_uart_disable_rx_irq(dev);
    r->imsc = 0x7f1;

    // Wait till UART is not busy anymore
    pl011_uart_wait_busy(dev);

    // Disable FIFO
    pl011_uart_disable_fifo(dev);

    // Set control register (CR)
    pl011_uart_cr_configure(dev);

    // Set line control register (LCRH)
    pl011_uart_lcrh_configure(dev);

    // Set baudrate divider
    pl011_uart_baudrate_div_configure(dev);

    /* NOTE: enabling FIFOs will cause an unintended
     * side effect for SerialServer. If enabled, a
     * receive interrupt is only triggered when
     * receive FIFO reaches the threshold level
     * set in IFLS register (default 1/8 full).
     *
     * This causes the SerialServer to receive
     * characters only after level is reached,
     * and that makes it unusable.
     *
     * Leaving FIFOs disabled will make them 1-byte
     * FIFOs, effectively mimicking the behavior
     * of mini-UART.
     *
     */
    // Enable FIFO
    //pl011_uart_enable_fifo(dev);

    // Enable UART
    pl011_uart_enable(dev);

    // Enable RX interrupt
    pl011_uart_enable_rx_irq(dev);

    return 0;
}

static void read_callback(ps_chardevice_t* device, enum chardev_status stat,
              size_t bytes_transfered, void* token){
    struct uart_token* t;
    t = (struct uart_token*)token;
    /* We might get a short read due to a timeout. */
    t->cur_bytes += bytes_transfered;
    t->buf += bytes_transfered;
}

int my_uart_read(char *c, int rsize)
{
    struct uart_token token;

    token.cur_bytes = 0;
    token.req_bytes = rsize;
    token.buf = c;
    if(ps_cdev_read(&serial_device, token.buf, token.req_bytes, &read_callback, &token) < 0){
        printf("Error reading from UART\n");
        return -1;
    }

    return token.cur_bytes;
}

int my_serial_init(enum chardev_id id, void *vaddr, mux_sys_t *mux_sys,
                       clk_t *clk_src, ps_chardevice_t *dev)
{
    dev->id         = id;
    dev->vaddr      = (void *)vaddr; // use real base address
    dev->read       = &my_uart_read;
    // dev->write      = &uart_write;
    dev->handle_irq = &pl011_uart_handle_irq;
    // dev->irqs       = defn->irqs;
    // dev->ioops      = *ops;
    // dev->flags      = SERIAL_AUTO_CR;

    pl011_uart_configure(dev);

    return 0;
}


void pre_init() {
    puts("Initializing UART Driver");
    if (my_serial_init(BCM2xxx_UART3, vaddr, NULL, NULL, &serial_device)) {
        printf("Failed to init UARt");
        while(1);
    }

    serial_configure(&serial_device, 115200, 8, PARITY_NONE, 1);
    
    interrupt_reg_callback(&interrupt_event, &serial_device);
}



int run() {
    while(1) {

    }
    return 0;
}
