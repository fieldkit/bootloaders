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

class ClassicLed {
private:
    volatile uint8_t value_ = 0;
    volatile uint8_t target_ = 0;
    volatile int8_t direction_ = 0;

public:
    void begin() {
        PORT->Group[BOARD_LED_PORT].DIRSET.reg = (1 << BOARD_LED_PIN);

        value_ = 0;
        target_ = 20;
        direction_ = 1;
    }

    void tick() {
        if (value_ == 0) {
            on();
        }

        value_++;
        if (value_ == 50) {
            value_ = 0;
        }

        if (value_ == target_) {
            off();
        }
    }

    void on() {
        PORT->Group[BOARD_LED_PORT].OUTSET.reg = (1 << BOARD_LED_PIN);
    }

    void off() {
        PORT->Group[BOARD_LED_PORT].OUTCLR.reg = (1 << BOARD_LED_PIN);
    }

};

static ClassicLed classic_led;

#if defined(FK_BOOTLOADER_LARGE)

#include <Adafruit_NeoPixel.h>

class NeoPixelLed {
private:
    Adafruit_NeoPixel pixel_{ 1, A3, NEO_GRB + NEO_KHZ400 };
    volatile uint8_t value_;
    volatile int8_t direction_;

public:
    static inline constexpr uint32_t get_color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
    }

    void begin() {
        value_ = 10;
        direction_ = 1;

        pixel_.updateLength(1);
        pixel_.updateType(NEO_GRB + NEO_KHZ400);
        pixel_.setPin(A3);
        pixel_.begin();
        pixel_.setBrightness(4);
        pixel_.setPixelColor(0, 0);
        pixel_.show();
    }

    void tick() {
        value_ += direction_;

        if (value_ > 240 || value_ < 10) {
            direction_ = -direction_;
        }

        pixel_.setPixelColor(0, get_color(value_, value_, value_));
        pixel_.show();
    }

    void off() {
        begin();
    }
};

static NeoPixelLed neopixel_led;

#endif

extern "C" {

void LED_init(void) {
    #if defined(BOARD_LED_PORT)
    classic_led.begin();
    #endif

    #if defined(FK_BOOTLOADER_LARGE)
    neopixel_led.begin();
    #endif
}

void LED_pulse() {
    #if defined(BOARD_LED_PORT)
    classic_led.tick();
    #endif

    #if defined(FK_BOOTLOADER_LARGE)
    neopixel_led.tick();
    #endif
}

void LED_off(void) {
    #if defined(BOARD_LED_PORT)
    classic_led.off();
    #endif

    #if defined(FK_BOOTLOADER_LARGE)
    neopixel_led.off();
    #endif
}

}
