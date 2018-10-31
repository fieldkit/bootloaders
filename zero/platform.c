#include "wiring.h"
#include "platform.h"
#include "serial5.h"
#include "board_driver_spi.h"

static volatile uint32_t uptime = 0;

static volatile bool initialized = false;

/**
 * I wish these were just standardized. FK-509
 */
board_configuration_t possible_boards[FK_NUMBER_OF_POSSIBLE_BOARDS] = {
    { 25u, 26u, 12u,  7u,  5u }, // Core
    {  8u,  6u,  0u,  0u,  5u }, // Sonar
    { 12u,  5u,  0u,  0u,  0u }, // Atlas
    {  8u,  5u,  0u,  0u,  0u }, // Weather
};

void board_prepare(board_configuration_t *cfg) {
    if (cfg->rfm95_cs > 0) {
        pinMode(cfg->rfm95_cs, OUTPUT);
        digitalWrite(cfg->rfm95_cs, HIGH);
    }

    if (cfg->sd_cs > 0) {
        pinMode(cfg->sd_cs, OUTPUT);
        digitalWrite(cfg->sd_cs, HIGH);
    }

    if (cfg->wifi_cs > 0) {
        pinMode(cfg->wifi_cs, OUTPUT);
        digitalWrite(cfg->wifi_cs, HIGH);
    }

    if (cfg->flash_cs > 0) {
        pinMode(cfg->flash_cs, OUTPUT);
        digitalWrite(cfg->flash_cs, HIGH);
    }

    if (cfg->periph_enable > 0) {
        pinMode(cfg->periph_enable, OUTPUT);
        digitalWrite(cfg->periph_enable, LOW);
        busy_delay(100);
        digitalWrite(cfg->periph_enable, HIGH);
        busy_delay(100);
    }
}

void platform_setup() {
    serial5_open();

    if (!initialized) {
        serial5_println("\n\nStart!\n");
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
