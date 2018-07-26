#ifndef _BOARD_DRIVER_SPI_H_
#define _BOARD_DRIVER_SPI_H_

#include <stdio.h>
#include <stdbool.h>
#include <sam.h>

void spi_open();

uint8_t spi_begin();

uint8_t spi_transfer(uint8_t data);

uint16_t spi_transfer_word(uint16_t w);

uint8_t spi_transfer_block(void *ptr, size_t n);

uint8_t spi_end();

void spi_close();

#endif // _BOARD_DRIVER_SPI_H_
