#include <stdarg.h>

#include "serial5.h"
#include "board_driver_serial.h"

#define COMM_USART_MODULE                 SERCOM5
#define COMM_USART_BUS_CLOCK_INDEX        PM_APBCMASK_SERCOM5
#define COMM_USART_PER_CLOCK_INDEX        GCLK_CLKCTRL_ID_SERCOM5_CORE_Val
#define COMM_USART_PAD_SETTINGS           UART_RX_PAD3_TX_PAD2
#define COMM_USART_PAD3                   PINMUX_PB23D_SERCOM5_PAD3
#define COMM_USART_PAD2                   PINMUX_PB22D_SERCOM5_PAD2
#define COMM_USART_PAD1                   PINMUX_UNUSED
#define COMM_USART_PAD0                   PINMUX_UNUSED

#define SERCOM_NVIC_PRIORITY ((1<<__NVIC_PRIO_BITS) - 1)

static uint8_t opened = 0;

void serial5_open() {
    uint32_t port;
    uint32_t pin;

    // Start the Software Reset
    COMM_USART_MODULE->USART.CTRLA.bit.SWRST = 1 ;

    while ( COMM_USART_MODULE->USART.CTRLA.bit.SWRST || COMM_USART_MODULE->USART.SYNCBUSY.bit.SWRST )
    {
        // Wait for both bits Software Reset from CTRLA and SYNCBUSY coming back to 0
    }

    //Reset (with 0) the STATUS register
    COMM_USART_MODULE->USART.STATUS.reg = SERCOM_USART_STATUS_RESETVALUE;

    if (COMM_USART_PAD2 != PINMUX_UNUSED)
    {
        /* Mask 6th bit in pin number to check whether it is greater than 32 i.e., PORTB pin */
        port = (COMM_USART_PAD2 & 0x200000) >> 21;
        pin = COMM_USART_PAD2 >> 16;
        PORT->Group[port].PINCFG[(pin - (port*32))].bit.PMUXEN = 1;
        PORT->Group[port].PMUX[(pin - (port*32))/2].reg &= ~(0xF << (4 * (pin & 0x01u)));
        PORT->Group[port].PMUX[(pin - (port*32))/2].reg |= (COMM_USART_PAD2 & 0xFF) << (4 * (pin & 0x01u));
    }

    if (COMM_USART_PAD3 != PINMUX_UNUSED)
    {
        /* Mask 6th bit in pin number to check whether it is greater than 32 i.e., PORTB pin */
        port = (COMM_USART_PAD3 & 0x200000) >> 21;
        pin = COMM_USART_PAD3 >> 16;
        PORT->Group[port].PINCFG[(pin - (port*32))].bit.PMUXEN = 1;
        PORT->Group[port].PMUX[(pin - (port*32))/2].reg &= ~(0xF << (4 * (pin & 0x01u)));
        PORT->Group[port].PMUX[(pin - (port*32))/2].reg |= (COMM_USART_PAD3 & 0xFF) << (4 * (pin & 0x01u));
    }

    // Setting NVIC
    NVIC_EnableIRQ(SERCOM5_IRQn);
    NVIC_SetPriority(SERCOM5_IRQn, SERCOM_NVIC_PRIORITY);  /* set Priority */

    /* Enable clock for BOOT_USART_MODULE */
    PM->APBCMASK.reg |= COMM_USART_BUS_CLOCK_INDEX;

    /* Set GCLK_GEN0 as source for GCLK_ID_SERCOMx_CORE */
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( COMM_USART_PER_CLOCK_INDEX ) | // Generic Clock 0 (SERCOMx)
        GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
        GCLK_CLKCTRL_CLKEN ;

    // enableDataRegisterEmptyInterruptUART
    COMM_USART_MODULE->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
        /* Wait for synchronization */
    }

    /* Baud rate 115200 - clock 48MHz -> BAUD value-63018 */
    uart_basic_init(COMM_USART_MODULE, 63018, COMM_USART_PAD_SETTINGS);

    opened = true;
}

void serial5_close() {
    if (opened) {
        uart_disable(COMM_USART_MODULE);
        opened = false;
    }
}

void serial5_putc(uint8_t value) {
    if (!opened) {
        return;
    }

    uart_write_byte(COMM_USART_MODULE, value);
}

void serial5_printf(const char *f, ...) {
    if (!opened) {
        return;
    }

    va_list args;
    va_start(args, f);
    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), f, args);
    va_end(args);

    for (const char *p = buffer; *p != 0; p++) {
        serial5_putc(*p);
    }
}

void serial5_flush() {
    if (opened) {
        uart_flush(COMM_USART_MODULE);
    }
}
