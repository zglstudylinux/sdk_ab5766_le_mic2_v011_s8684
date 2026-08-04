#ifndef _API_FLASH_H_
#define _API_FLASH_H_


typedef uint8_t flash_id_t[16];
typedef uint8_t flash_cap_id_t[4];


/**
  * @brief  Read data from the chip's built-in flash
  * @param  buf
  * @param  flash address to read
  * @param  len
  */
uint os_spiflash_read(void *buf, u32 addr, uint len);

/**
  * @brief  Program data to the chip's built-in flash
  * @param  buf
  * @param  flash address to program to
  * @param  len, Cannot be written across pages, 256 bytes per page
  */
void os_spiflash_program(void *buf, u32 addr, uint len);

/**
  * @brief  Erase the chip's built-in flash
  * @param  flash base addresses that need to be erased, erase 4K bytes at a time
  */
void os_spiflash_erase(u32 addr);

/**
  * @brief  Get the chip's built-in flash id
  * @param  flash id buf
  */
void os_spiflash_id_get(flash_id_t id);

/**
  * @brief  Get the chip's built-in manufacture id and capacity id
  * @param  capacity id buf
  */
void os_spiflash_cap_id_get(flash_cap_id_t cap_id);

#endif

