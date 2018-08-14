#ifndef _FIRMWARE_HEADER_H_
#define _FIRMWARE_HEADER_H_

#include <sam.h>

#ifdef __cplusplus
#include <cinttypes>
#endif

#define FIRMWARE_VERSION_INVALID             ((uint32_t)-1)
#define FIRMWARE_HEADER_MODULE_MAXIMUM       (64)
#define FIRMWARE_HEADER_TAG_MAXIMUM          (64)

extern uint32_t __ProgramBegin__;
extern uint32_t __FirmwareState__;

#define FIRMWARE_NVM_PROGRAM_ADDRESS         ((uint32_t)(&__ProgramBegin__))
#define FIRMWARE_NVM_HEADER_ADDRESS          ((uint32_t)(&__FirmwareState__))

/*
 * If BOOT_STATE_ADDRESS is defined the bootloader is started by
 * quickly tapping two times on the reset button.
 * BOOT_STATE_ADDRESS must point to a free SRAM cell that must not
 * be touched from the loaded application.
 */
#if false
extern uint32_t __StackTop;
#define BOOT_STATE_ADDRESS                   (&__StackTop)
#endif
#define BOOT_STATE_ADDRESS_ORIGINAL          (0x20007FFCul)
#define BOOT_STATE_DATA                      (*((volatile uint32_t *) BOOT_STATE_ADDRESS_ORIGINAL))
#define BOOT_STATE_VALUE_FLASH               (0x07688924)
#define BOOT_STATE_VALUE_DOUBLE_TAP          (0x07738135)

typedef struct firmware_header_t {
    uint32_t version;
    uint32_t time;
    uint32_t size;
    char module[FIRMWARE_HEADER_MODULE_MAXIMUM];
    char etag[FIRMWARE_HEADER_TAG_MAXIMUM];
} firmware_header_t;

static inline void firmware_self_flash() {
    BOOT_STATE_DATA = BOOT_STATE_VALUE_FLASH;
    NVIC_SystemReset();
}

#endif
