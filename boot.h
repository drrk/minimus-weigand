#ifndef _BOOT_H
#define _BOOT_H

#define MAGIC_BOOT_KEY 0x4AC59ACE
#define FLASH_SIZE_BYTES 0x8000
#define BOOTLOADER_SEC_SIZE_BYTES 4096
#define BOOTLOADER_START_ADDRESS (FLASH_SIZE_BYTES - BOOTLOADER_SEC_SIZE_BYTES)

void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void Jump_To_Bootloader(void);

#endif
