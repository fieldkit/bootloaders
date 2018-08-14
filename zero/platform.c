#include "wiring.h"
#include "platform.h"
#include "serial5.h"
#include "board_driver_spi.h"

static volatile uint32_t uptime = 0;
static volatile bool initialized = false;

void platform_setup() {
    pinMode(RFM95_PIN_CS, OUTPUT);
    pinMode(SD_PIN_CS, OUTPUT);
    pinMode(WIFI_PIN_CS, OUTPUT);
    pinMode(FLASH_PIN, OUTPUT);

    pinMode(PERIPH_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPH_ENABLE_PIN, LOW);
    busy_delay(500);
    digitalWrite(PERIPH_ENABLE_PIN, HIGH);
    busy_delay(500);

    digitalWrite(RFM95_PIN_CS, HIGH);
    digitalWrite(SD_PIN_CS, HIGH);
    digitalWrite(WIFI_PIN_CS, HIGH);
    digitalWrite(FLASH_PIN, HIGH);

    serial5_open();

    if (!initialized) {
        serial5_println("\n\nStart!");
        initialized = true;
    }

    spi_open();
}

void busy_delay(uint32_t ms) {
    /* Wait 0.5sec to see if the user tap reset again.
     * The loop value is based on SAMD21 default 1MHz clock @ reset.
     */
    for (uint32_t i = 0; i < 12500 * ms; i++) {
        /* force compiler to not optimize this... */
        __asm__ __volatile__("");
    }
}

void delay(uint32_t ms) {
    if (ms == 0) {
        return;
    }

    uint32_t began = millis();

    do {
        __asm__ __volatile__("");
    }
    while (millis() - began < ms);
}

uint32_t millis() {
    return uptime;
}

void platform_default_sys_tick() {
    uptime++;
}

void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));

void __cxa_pure_virtual(void) {
    while (1) {
    }
}

void __cxa_deleted_virtual(void) {
    while (1) {
    }
}
