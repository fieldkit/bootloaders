#include "wiring.h"
#include "platform.h"
#include "serial5.h"
#include "board_definitions.h"
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


static uint8_t disabled_pins[] = {
    /* SPI_PIN_MISO */ 22,
    /* SPI_PIN_MOSI */ 23,
    /* SPI_PIN_SCK */ 24,
    /* I2C_PIN_SDA1 */ 20,
    /* I2C_PIN_SCL1 */ 21,
    /* I2C_PIN_SDA2 */ 4,
    /* I2C_PIN_SCL2 */ 3,
    /* WIFI_PIN_CS */ 7,
    /* FLASH_PIN_CS */ (26u),
    /* SD_PIN_CS */ 12,
    /* RFM95_PIN_CS */ 5,
    0
};

void platform_board_disable() {
    for (size_t i = 0; disabled_pins[i] > 0; ++i) {
        pinMode(i, INPUT);
    }
}

void platform_setup() {
    serial5_open();

    if (!initialized) {
        serial5_println("\n\nStart!\n");
        initialized = true;
    }

    platform_board_disable();
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

// Interrupt-compatible version of micros
// Theory: repeatedly take readings of SysTick counter, millis counter and SysTick interrupt pending flag.
// When it appears that millis counter and pending is stable and SysTick hasn't rolled over, use these
// values to calculate micros. If there is a pending SysTick, add one to the millis counter in the calculation.
uint32_t micros(void) {
    uint32_t ticks, ticks2;
    uint32_t pend, pend2;
    uint32_t count, count2;

    ticks2 = SysTick->VAL;
    pend2  = !!(SCB->ICSR & SCB_ICSR_PENDSTSET_Msk);
    count2 = uptime;

    do
    {
        ticks = ticks2;
        pend = pend2;
        count = count2;
        ticks2 = SysTick->VAL;
        pend2  = !!(SCB->ICSR & SCB_ICSR_PENDSTSET_Msk);
        count2 = uptime;
    } while ((pend != pend2) || (count != count2) || (ticks < ticks2));

    return ((count + pend) * 1000) + (((SysTick->LOAD  - ticks) * (1048576 / (VARIANT_MCK / 1000000))) >> 20);
    // this is an optimization to turn a runtime division into two compile-time divisions and
    // a runtime multiplication and shift, saving a few cycles
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
