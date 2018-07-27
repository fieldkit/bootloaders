#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include <stdlib.h>
#include <sam.h>

typedef struct firmware_header_t {
    uint32_t version;
    uint32_t position;
    uint32_t size;
    uint8_t reserved[64 - (4 * 3)];
} firmware_header_t;

uint8_t firmware_check_before_launch();

#endif // _FIRMWARE_H_
