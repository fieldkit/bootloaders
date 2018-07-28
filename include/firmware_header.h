#ifndef _FIRMWARE_HEADER_H_
#define _FIRMWARE_HEADER_H_

#define FLASH_FIRMWARE_BANK_ADDRESS          (1572864)
#define FLASH_FIRMWARE_BANK_SIZE             (256 * 1024)
#define FLASH_FIRMWARE_BANK_1_ADDRESS        (1572864)
#define FLASH_FIRMWARE_BANK_2_ADDRESS        (1572864 + FLASH_FIRMWARE_BANK_SIZE)
#define FLASH_FIRMWARE_BANK_1_HEADER_ADDRESS (FLASH_FIRMWARE_BANK_1_ADDRESS + FLASH_FIRMWARE_BANK_SIZE - sizeof(firmware_header_t))
#define FLASH_FIRMWARE_BANK_2_HEADER_ADDRESS (FLASH_FIRMWARE_BANK_2_ADDRESS + FLASH_FIRMWARE_BANK_SIZE - sizeof(firmware_header_t))

#define FIRMWARE_VERSION_INVALID             ((uint32_t)-1)
#define FIRMWARE_HEADER_TAG_MAXIMUM          (32)

#define FIRMWARE_NVM_PROGRAM_ADDRESS         (0x4000)
#define FIRMWARE_NVM_HEADER_ADDRESS          ((void *)262144 - 2048)

typedef struct firmware_header_t {
    uint32_t version;
    uint32_t position;
    uint32_t size;
    char etag[FIRMWARE_HEADER_TAG_MAXIMUM];
    uint8_t reserved[64 - (4 * 3) - FIRMWARE_HEADER_TAG_MAXIMUM];
} firmware_header_t;

#endif
