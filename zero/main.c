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
#include "platform.h"
#include "firmware.h"
#include "firmware_header.h"

extern uint32_t __sketch_vectors_ptr;

extern void board_initialize(void);

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
    serial5_println("No program: 0x%x (0x%x)", __sketch_vectors_ptr, &__sketch_vectors_ptr);
    serial5_flush();
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
    serial5_println("No vector table: 0x%x (0x%x)", __sketch_vectors_ptr, &__sketch_vectors_ptr);
    serial5_flush();
    return;
  }

  uint8_t self_flash_allowed = false;

  if (PM->RCAUSE.bit.POR)
  {
    /* On power-on initialize double-tap */
    BOOT_STATE_DATA = 0;
  }
  else
  {
    if (BOOT_STATE_DATA == BOOT_STATE_VALUE_DOUBLE_TAP)
    {
      /* Second tap, stay in bootloader */
      serial5_println("BootSate == 0x%x, stay in bootloader!", BOOT_STATE_VALUE_DOUBLE_TAP);
      serial5_flush();
      BOOT_STATE_DATA = 0;
      return;
    }
    if (BOOT_STATE_DATA == BOOT_STATE_VALUE_FLASH)
    {
      serial5_println("BootSate == 0x%x, self flash allowed!", BOOT_STATE_VALUE_FLASH);
      serial5_flush();
      BOOT_STATE_DATA = 0;
      self_flash_allowed = true;
    }

    /* First tap */
    BOOT_STATE_DATA = BOOT_STATE_VALUE_DOUBLE_TAP;

    delay(500);

    /* Timeout happened, continue boot... */
    BOOT_STATE_DATA = 0;
  }

  #ifdef FK_BOOTLOADER_ENABLE_FLASH
  if (self_flash_allowed) {
    firmware_check_before_launch();
  }
  #endif
  serial5_println("Program: 0x%x (0x%x)", __sketch_vectors_ptr, &__sketch_vectors_ptr);
  serial5_flush();

  LED_off();

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

static uint32_t began = 0;

int loop() {
    if (began == 0) {
        serial5_println("Waiting");
        began = millis() + 1;
    }
    else if (millis() > began && millis() - began > INACTIVITY_TIMEOUT) {
        serial5_println("Reboot (%lu)", millis() - began);
        serial5_flush();
        NVIC_SystemReset();
    }

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
  board_initialize();

  /* Start the sys tick (1 ms) */
  SysTick_Config(VARIANT_MCK / 1000);

  #ifdef FK_BOOTLOADER_ENABLE_FLASH
  platform_setup();
  #endif

  /* Jump in application if condition is satisfied */
  check_start_application();

  /* We have determined we should stay in the monitor. */
  /* System initialization */
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

  began = 0;

  /* Wait for a complete enum on usb or a '#' char on serial line */
  while (1)
  {
    loop();
#if SAM_BA_INTERFACE == SAM_BA_USBCDC_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
    if (pCdc->IsConfigured(pCdc) != 0)
    {
      main_b_cdc_enable = true;
    }

    /* Check if a USB enumeration has succeeded and if comm port has been opened */
    if (main_b_cdc_enable)
    {
      sam_ba_monitor_init(SAM_BA_INTERFACE_USBCDC);
      sam_ba_monitor_prepare();
      /* SAM-BA on USB loop */
      while( 1 )
      {
        sam_ba_monitor_loop();
        loop();
      }
    }
#endif

#if SAM_BA_INTERFACE == SAM_BA_UART_ONLY  ||  SAM_BA_INTERFACE == SAM_BA_BOTH_INTERFACES
    /* Check if a '#' has been received */
    if (!main_b_cdc_enable && serial_sharp_received())
    {
      sam_ba_monitor_init(SAM_BA_INTERFACE_USART);
      sam_ba_monitor_prepare();
      /* SAM-BA on Serial loop */
      while(1)
      {
        sam_ba_monitor_loop();
        loop();
      }
    }
#endif
  }
}

void SysTick_Handler(void)
{
  platform_default_sys_tick();

  LED_pulse();

  sam_ba_monitor_sys_tick();
}
