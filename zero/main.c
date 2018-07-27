/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Copyright (c) 2015 Atmel Corporation/Thibaut VIARD.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <sam.h>
#include "sam_ba_monitor.h"
#include "sam_ba_serial.h"
#include "board_definitions.h"
#include "board_driver_led.h"
#include "sam_ba_usb.h"
#include "sam_ba_cdc.h"

#include "serial5.h"
#include "board_driver_spi.h"
#include "flash_memory.h"
#include "nvm_memory.h"

extern uint32_t __sketch_vectors_ptr; // Exported value from linker script
extern void board_init(void);

#if (defined DEBUG) && (DEBUG == 1)
volatile uint32_t* pulSketch_Start_Address;
#endif

static volatile bool main_b_cdc_enable = false;

/**
 * \brief Check the application startup condition
 *
 */
static void check_start_application(void)
{
//  LED_init();
//  LED_off();

#if (!defined DEBUG) || ((defined DEBUG) && (DEBUG == 0))
uint32_t* pulSketch_Start_Address;
#endif

  /*
   * Test sketch stack pointer @ &__sketch_vectors_ptr
   * Stay in SAM-BA if value @ (&__sketch_vectors_ptr) == 0xFFFFFFFF (Erased flash cell value)
   */
  if (__sketch_vectors_ptr == 0xFFFFFFFF)
  {
    /* Stay in bootloader */
    return;
  }

  /*
   * Load the sketch Reset Handler address
   * __sketch_vectors_ptr is exported from linker script and point on first 32b word of sketch vector table
   * First 32b word is sketch stack
   * Second 32b word is sketch entry point: Reset_Handler()
   */
  pulSketch_Start_Address = &__sketch_vectors_ptr ;
  pulSketch_Start_Address++ ;

  /*
   * Test vector table address of sketch @ &__sketch_vectors_ptr
   * Stay in SAM-BA if this function is not aligned enough, ie not valid
   */
  if ( ((uint32_t)(&__sketch_vectors_ptr) & ~SCB_VTOR_TBLOFF_Msk) != 0x00)
  {
    /* Stay in bootloader */
    return;
  }

#if defined(BOOT_DOUBLE_TAP_ADDRESS)
  #define DOUBLE_TAP_MAGIC 0x07738135
  if (PM->RCAUSE.bit.POR)
  {
    /* On power-on initialize double-tap */
    BOOT_DOUBLE_TAP_DATA = 0;
  }
  else
  {
    if (BOOT_DOUBLE_TAP_DATA == DOUBLE_TAP_MAGIC)
    {
      /* Second tap, stay in bootloader */
      BOOT_DOUBLE_TAP_DATA = 0;
      return;
    }

    /* First tap */
    BOOT_DOUBLE_TAP_DATA = DOUBLE_TAP_MAGIC;

    /* Wait 0.5sec to see if the user tap reset again.
     * The loop value is based on SAMD21 default 1MHz clock @ reset.
     */
    for (uint32_t i=0; i<125000 * 50; i++) /* 500ms */
      /* force compiler to not optimize this... */
      __asm__ __volatile__("");

    /* Timeout happened, continue boot... */
    BOOT_DOUBLE_TAP_DATA = 0;
  }
#endif

/*
#if defined(BOOT_LOAD_PIN)
  volatile PortGroup *boot_port = (volatile PortGroup *)(&(PORT->Group[BOOT_LOAD_PIN / 32]));
  volatile bool boot_en;

  // Enable the input mode in Boot GPIO Pin
  boot_port->DIRCLR.reg = BOOT_PIN_MASK;
  boot_port->PINCFG[BOOT_LOAD_PIN & 0x1F].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
  boot_port->OUTSET.reg = BOOT_PIN_MASK;
  // Read the BOOT_LOAD_PIN status
  boot_en = (boot_port->IN.reg) & BOOT_PIN_MASK;

  // Check the bootloader enable condition
  if (!boot_en)
  {
    // Stay in bootloader
    return;
  }
#endif
*/

//  LED_on();

  serial5_printf("Program: 0x%x (0x%x)\n\r", __sketch_vectors_ptr, &__sketch_vectors_ptr);
  serial5_flush();

  // return;

  /* Rebase the Stack Pointer */
  __set_MSP( (uint32_t)(__sketch_vectors_ptr) );

  /* Rebase the vector table base address */
  SCB->VTOR = ((uint32_t)(&__sketch_vectors_ptr) & SCB_VTOR_TBLOFF_Msk);

  /* Jump to application Reset Handler in the application */
  asm("bx %0"::"r"(*pulSketch_Start_Address));
}

#if DEBUG_ENABLE
#	define DEBUG_PIN_HIGH 	port_pin_set_output_level(BOOT_LED, 1)
#	define DEBUG_PIN_LOW 	port_pin_set_output_level(BOOT_LED, 0)
#else
#	define DEBUG_PIN_HIGH 	do{}while(0)
#	define DEBUG_PIN_LOW 	do{}while(0)
#endif

typedef struct firmware_header_t {
    uint32_t position;
    uint32_t size;
} firmware_header_t;

int firmware_flash(flash_memory_t *fmem) {
    firmware_header_t header = {
        1835008,
        24260
    };

    uint32_t PageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
    uint32_t page_size = PageSizes[NVMCTRL->PARAM.bit.PSZ];
    uint32_t pages = NVMCTRL->PARAM.bit.NVMP;
    uint32_t flash_size = page_size * pages;

    uint8_t buffer[1024];

    serial5_println("Flash: page-size=%d pages=%d", page_size, pages);

    uint32_t writing = 0x4000;

    serial5_println("Flash: Erasing (0x%x)", writing);

    nvm_erase_after(writing);

    uint32_t bytes = 0;
    uint32_t reading = header.position;

    while (bytes < header.size) {
        serial5_println("Flash: Writing 0x%x -> 0x%x -> 0x%x (%d)", reading, buffer, writing, bytes);
        flash_read(fmem, reading, buffer, sizeof(buffer));

        nvm_write((uint32_t *)writing, (uint32_t *)buffer, sizeof(buffer) / 4);

        reading += sizeof(buffer);
        writing += sizeof(buffer);
        bytes += sizeof(buffer);
    }

    return 0;
}

int firmware_check(bool flash) {
    serial5_open();

    serial5_println("");
    serial5_println("");

    serial5_println("Bootloader Ready");

    serial5_println("Opening SPI...");

    spi_open();

    serial5_println("Opening Flash...");

    flash_memory_t fmem;
    flash_open(&fmem, 26);
    uint8_t buffer[8] = { 0 };

    flash_read(&fmem, 65536, buffer, sizeof(buffer));

    serial5_printf("Data: ");
    for (uint8_t i = 0; i < sizeof(buffer); ++i) {
        serial5_printf("%x ", buffer[i]);
    }
    serial5_println("");
    serial5_println("Done");

    if (flash) {
        firmware_flash(&fmem);
    }

    flash_close(&fmem);

    serial5_flush();

    return 0;
}

int firmware_check_before_launch() {
    board_init();

    firmware_check(false);

    return 0;
}

/**
 *  \brief SAMD21 SAM-BA Main loop.
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
#if SAM_BA_INTERFACE == SAM_BA_USBCDC_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
  P_USB_CDC pCdc;
#endif
  DEBUG_PIN_HIGH;

  /* Check for a firmware update. */
  firmware_check_before_launch();

  /* Jump in application if condition is satisfied */
  check_start_application();

  /* We have determined we should stay in the monitor. */
  /* System initialization */
  serial5_close();
  board_init();
  __enable_irq();

#if SAM_BA_INTERFACE == SAM_BA_UART_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
  /* UART is enabled in all cases */
  serial_open();
#endif

#if SAM_BA_INTERFACE == SAM_BA_USBCDC_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
  pCdc = usb_init();
#endif

  DEBUG_PIN_LOW;

  /* Initialize LEDs */
  LED_init();
  LEDRX_init();
  LEDRX_off();
  LEDTX_init();
  LEDTX_off();

  /* Start the sys tick (1 ms) */
  SysTick_Config(1000);

  firmware_check(false);

  serial5_println("Waiting...");
  serial5_flush();

  /* Wait for a complete enum on usb or a '#' char on serial line */
  while (1)
  {
#if SAM_BA_INTERFACE == SAM_BA_USBCDC_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
    if (pCdc->IsConfigured(pCdc) != 0)
    {
      main_b_cdc_enable = true;
    }

    /* Check if a USB enumeration has succeeded and if comm port has been opened */
    if (main_b_cdc_enable)
    {
      sam_ba_monitor_init(SAM_BA_INTERFACE_USBCDC);
      /* SAM-BA on USB loop */
      while( 1 )
      {
        sam_ba_monitor_run();
      }
    }
#endif

#if SAM_BA_INTERFACE == SAM_BA_UART_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
    /* Check if a '#' has been received */
    if (!main_b_cdc_enable && serial_sharp_received())
    {
      sam_ba_monitor_init(SAM_BA_INTERFACE_USART);
      /* SAM-BA on Serial loop */
      while(1)
      {
        sam_ba_monitor_run();
      }
    }
#endif
  }
}

void SysTick_Handler(void)
{
  LED_pulse();

  sam_ba_monitor_sys_tick();
}
