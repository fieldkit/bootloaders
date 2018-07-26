#include "flash_memory.h"
#include "board_driver_spi.h"
#include "serial5.h"
#include "wiring.h"

#define ID0_WINBOND	0xEF
#define ID0_SPANSION	0x01
#define ID0_MICRON	0x20
#define ID0_MACRONIX	0xC2
#define ID0_SST		0xBF

void flash_open(uint8_t cs) {
    char buf[16] = { 0 };

    spi_begin();
    digitalWrite(cs, LOW);
    spi_transfer(0x9F);
    buf[0] = spi_transfer(0); // manufacturer ID
    buf[1] = spi_transfer(0); // memory type
    buf[2] = spi_transfer(0); // capacity
    if (buf[0] == ID0_SPANSION) {
        buf[3] = spi_transfer(0); // ID-CFI
        buf[4] = spi_transfer(0); // sector size
    }
    digitalWrite(cs, HIGH);
    spi_end();

    serial5_printf("ID: %02X %02X %02X %02X %02X\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
}

void flash_read() {
}

void flash_close() {
}
