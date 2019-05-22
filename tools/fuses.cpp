#include <Arduino.h>

// https://github.com/microsoft/uf2-samdx1/blob/master/src/selfmain.c

#ifdef __SAMD21__
#define NVM_FUSE_ADDR NVMCTRL_AUX0_ADDRESS
#define exec_cmd(cmd)                                                          \
    do {                                                                       \
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;                            \
        NVMCTRL->ADDR.reg = (uint32_t)NVMCTRL_USER / 2;                        \
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;                    \
        while (NVMCTRL->INTFLAG.bit.READY == 0) {}                             \
    } while (0)
#endif
#ifdef __SAMD51__
#define NVM_FUSE_ADDR NVMCTRL_FUSES_BOOTPROT_ADDR
#define exec_cmd(cmd)                                                          \
    do {                                                                       \
        NVMCTRL->ADDR.reg = (uint32_t)NVMCTRL_USER;                        \
        NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | cmd;                    \
        while (NVMCTRL->STATUS.bit.READY == 0) {}                              \
    } while (0)
#endif

void setup() {
    #ifdef __SAMD21__
    while (!(NVMCTRL->INTFLAG.reg & NVMCTRL_INTFLAG_READY)) { }
    #endif
    #ifdef __SAMD51__
    while (NVMCTRL->STATUS.bit.READY == 0) { }
    #endif

    uint32_t fuses[2];
    fuses[0] = *((uint32_t *)NVM_FUSE_ADDR);
    fuses[1] = *(((uint32_t *)NVM_FUSE_ADDR) + 1);

    uint32_t bootprot = (fuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

    // os_printf("Fuse: %x\n", fuses[0]);
    // os_printf("Fuse: %x\n", fuses[1]);
    // os_printf("BOOTPROT: %d\n", bootprot);

    fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (15 << NVMCTRL_FUSES_BOOTPROT_Pos);

    #ifdef __SAMD21__
    NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;

    exec_cmd(NVMCTRL_CTRLA_CMD_EAR);
    exec_cmd(NVMCTRL_CTRLA_CMD_PBC);
    #endif
    #ifdef __SAMD51__
    NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

    exec_cmd(NVMCTRL_CTRLB_CMD_EP);
    exec_cmd(NVMCTRL_CTRLB_CMD_PBC);
    #endif

    *((uint32_t *)NVM_FUSE_ADDR) = fuses[0];
    *(((uint32_t *)NVM_FUSE_ADDR) + 1) = fuses[1];

    #ifdef __SAMD21__
    exec_cmd(NVMCTRL_CTRLA_CMD_WAP);
    #endif
    #ifdef __SAMD51__
    exec_cmd(NVMCTRL_CTRLB_CMD_WQW);
    #endif
}

void loop() {
}
