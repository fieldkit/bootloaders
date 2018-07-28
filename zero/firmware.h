#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include <stdlib.h>
#include <sam.h>

#define FIRMWARE_HEADER_TAG_MAXIMUM          (32)

typedef struct firmware_header_t {
    uint32_t version;
    uint32_t position;
    uint32_t size;
    char etag[FIRMWARE_HEADER_TAG_MAXIMUM];
    uint8_t reserved[64 - (4 * 3) - FIRMWARE_HEADER_TAG_MAXIMUM];
} firmware_header_t;

uint8_t firmware_check_before_launch();

uint8_t firmware_backups_erase();

#endif // _FIRMWARE_H_
