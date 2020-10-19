#include "contiki.h"
#include "contiki-conf.h"
#include "stdint.h"
/*
使用CC2530的Flash模拟EEPROM
*/


// Flash is partitioned into 8 banks of 32 KB or 16 pages.
#define HAL_FLASH_PAGE_PER_BANK    16
// Flash is constructed of 128 pages of 2 KB.
#define HAL_FLASH_PAGE_SIZE        2048
#define HAL_FLASH_WORD_SIZE        4

// CODE banks get mapped into the XDATA range 8000-FFFF.
#define HAL_FLASH_PAGE_MAP         0x8000

// The last 16 bytes of the last available page are reserved for flash lock bits.
// NV page definitions must coincide with segment declaration in project *.xcl file.
#if defined NON_BANKED
#define HAL_FLASH_LOCK_BITS        16
#define HAL_NV_PAGE_END            30
#define HAL_NV_PAGE_CNT            2
#else
#define HAL_FLASH_LOCK_BITS        16
#define HAL_NV_PAGE_END            126
#define HAL_NV_PAGE_CNT            6
#endif
/**************************************************************************************************
 * @fn          HalFlashRead
 *
 * @brief       This function reads 'cnt' bytes from the internal flash.
 *
 * input parameters
 *
 * @param       pg - A valid flash page number.
 * @param       offset - A valid offset into the page.
 * @param       buf - A valid buffer space at least as big as the 'cnt' parameter.
 * @param       cnt - A valid number of bytes to read.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashRead(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t cnt)
{
  // Calculate the offset into the containing flash bank as it gets mapped into XDATA.
  uint8_t *pData = (uint8_t *)(offset + HAL_FLASH_PAGE_MAP) +
                 ((pg % HAL_FLASH_PAGE_PER_BANK) * HAL_FLASH_PAGE_SIZE);
  uint8_t memctr = MEMCTR;  // Save to restore.



  pg /= HAL_FLASH_PAGE_PER_BANK;  // Calculate the flash bank from the flash page.


  DISABLE_INTERRUPTS();

  // Calculate and map the containing flash bank into XDATA.
  MEMCTR = (MEMCTR & 0xF8) | pg;

  while (cnt--)
  {
    *buf++ = *pData++;
  }

  MEMCTR = memctr;

  ENABLE_INTERRUPTS();
}
/**************************************************************************************************
 * @fn          HalFlashErase
 *
 * @brief       This function erases the specified page of the internal flash.
 *
 * input parameters
 *
 * @param       pg - A valid flash page number to erase.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashErase(uint8_t pg)
{
  uint8_t faddrh=FADDRH;
  DISABLE_INTERRUPTS();
  while(FCTL&0x80);
  FADDRH = pg * (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE / 256);
  FCTL |= 0x01;
  while(FCTL&0x80);
  ENABLE_INTERRUPTS();
  FADDRH=faddrh;
}

