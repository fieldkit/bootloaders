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

#include "board_driver_led.h"
#include "serial5.h"

volatile uint8_t ledValue = 10;
volatile int8_t ledDirection = 1;

#if defined(FK_BOOTLOADER_LARGE)

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixel{ 1, A3, NEO_GRB + NEO_KHZ400 };

#endif

extern "C" {

static inline constexpr uint32_t get_color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

void LED_init(void) {
    #if defined(BOARD_LED_PORT)
    PORT->Group[BOARD_LED_PORT].DIRSET.reg = (1 << BOARD_LED_PIN);
    #endif

    #if defined(FK_BOOTLOADER_LARGE)
    pixel.updateLength(1);
    pixel.updateType(NEO_GRB + NEO_KHZ400);
    pixel.setPin(A3);
    pixel.begin();
    pixel.setBrightness(4);
    pixel.setPixelColor(0, get_color(200, 32, 8));
    pixel.show();
    #endif
}

void LED_pulse() {
    ledValue += ledDirection;

    if (ledValue > 240 || ledValue < 10) {
        ledDirection = -ledDirection;
    }

    #if defined(FK_BOOTLOADER_LARGE)
    pixel.setPixelColor(0, get_color(ledValue, ledValue, ledValue));
    pixel.show();
    #endif
}

}
