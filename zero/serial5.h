#ifndef _FK_SERIAL5_H_
#define _FK_SERIAL5_H_

#include <stdio.h>
#include <stdbool.h>
#include <sam.h>

void serial5_open();

void serial5_close();

void serial5_putc(uint8_t value);

void serial5_printf(const char *ptr, ...);

void serial5_println(const char *ptr, ...);

void serial5_flush();

#endif
