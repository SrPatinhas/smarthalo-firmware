/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1f000, LENGTH = 0x61000
  RAM (rwx) :  ORIGIN = 0x20002128, LENGTH = 0xded8

  /** Location of boot settings */
  BOOTINFO (rw) : ORIGIN = 0x0007C000, LENGTH = 0x1000

  /** Location of bootloader setting in flash. */
  BOOTLOADER_SETTINGS (rw) : ORIGIN = 0x0007F000, LENGTH = 0x1000

}

SECTIONS
{
  /* Place the boot settings page in flash. */
  .bootInfo(NOLOAD) :
  {

  } > BOOTINFO
  /* Place the bootloader settings page in flash. */
  .bootloaderSettings(NOLOAD) :
  {

  } > BOOTLOADER_SETTINGS
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf5x_common.ld"
