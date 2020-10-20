#include "contiki.h"
#include "contiki-conf.h"
#include "stdint.h"
#include "haldma.h"
#include "halflash.h"

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
 * @fn          HalFlashWrite
 *
 * @brief       This function writes 'cnt' bytes to the internal flash.
 *
 * input parameters
 *
 * @param       addr - Valid HAL flash write address: actual addr / 4 and quad-aligned.
 * @param       buf - Valid buffer space at least as big as 'cnt' X 4.
 * @param       cnt - Number of 4-byte blocks to write.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashWrite(uint16_t addr, uint8_t *buf, uint16_t cnt)
{
  halDMADesc_t *ch = HAL_NV_DMA_GET_DESC();

  HAL_DMA_SET_SOURCE(ch, buf);
  HAL_DMA_SET_DEST(ch, &FWDATA);
  HAL_DMA_SET_VLEN(ch, HAL_DMA_VLEN_USE_LEN);
  HAL_DMA_SET_LEN(ch, (cnt * HAL_FLASH_WORD_SIZE));
  HAL_DMA_SET_WORD_SIZE(ch, HAL_DMA_WORDSIZE_BYTE);
  HAL_DMA_SET_TRIG_MODE(ch, HAL_DMA_TMODE_SINGLE);
  HAL_DMA_SET_TRIG_SRC(ch, HAL_DMA_TRIG_FLASH);
  HAL_DMA_SET_SRC_INC(ch, HAL_DMA_SRCINC_1);
  HAL_DMA_SET_DST_INC(ch, HAL_DMA_DSTINC_0);
  // The DMA is to be polled and shall not issue an IRQ upon completion.
  HAL_DMA_SET_IRQ(ch, HAL_DMA_IRQMASK_DISABLE);
  HAL_DMA_SET_M8( ch, HAL_DMA_M8_USE_8_BITS);
  HAL_DMA_SET_PRIORITY(ch, HAL_DMA_PRI_HIGH);
  HAL_DMA_CLEAR_IRQ(HAL_NV_DMA_CH);
  HAL_DMA_ARM_CH(HAL_NV_DMA_CH);

  FADDRL = (uint8)addr;
  FADDRH = (uint8)(addr >> 8);
  FCTL |= 0x02;         // Trigger the DMA writes.
  while (FCTL & 0x80);  // Wait until writing is done.

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

