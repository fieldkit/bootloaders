#ifndef _DRIVER_NVM_MEMORY_H_
#define _DRIVER_NVM_MEMORY_H_

#include <sam.h>

void nvm_erase_row(uint32_t *d);

void nvm_erase_after(uint32_t address);

void nvm_write(uint32_t *d, uint32_t *src, uint32_t nwords);

#endif // _DRIVER_NVM_MEMORY_H_
