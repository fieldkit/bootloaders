#ifndef _FIRMWARE_HEADER_H_
#define _FIRMWARE_HEADER_H_

#ifdef __cplusplus
#include <cinttypes>
#endif

#define FIRMWARE_VERSION_INVALID             ((uint32_t)-1)
#define FIRMWARE_HEADER_MODULE_MAXIMUM       (64)
#define FIRMWARE_HEADER_TAG_MAXIMUM          (64)

#define FIRMWARE_NVM_PROGRAM_ADDRESS         (0x8000)
#define FIRMWARE_NVM_HEADER_ADDRESS          ((uint8_t *)262144 - 2048)

typedef struct firmware_header_t {
    uint32_t version;
    uint32_t time;
    uint32_t size;
    char module[FIRMWARE_HEADER_MODULE_MAXIMUM];
    char etag[FIRMWARE_HEADER_TAG_MAXIMUM];
} firmware_header_t;

#endif
