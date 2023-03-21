/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <autoconf.h>
#include <camkes.h>
#include <stdint.h>
#include <stdio.h>

#define IO_BASE_ADDR    (unsigned int)mem
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

void uart__init()
{
    printf("uart port init\n");
}


void uart_put_char1(char c)
{
    while (*(uint8_t*)(IO_BASE_ADDR + 0x18) & FR_TXFF);
    *(uint8_t*)(IO_BASE_ADDR) = c & 0xff;
}
