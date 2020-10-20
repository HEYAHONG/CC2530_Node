#include "contiki.h"
#include "contiki-conf.h"
#include "stdint.h"
#include "haldma.h"
#include "halflash.h"
/*
使用CC2530的Flash模拟EEPROM,所有操作都必须四字节对其。
*/

#warning 启用了Flash模拟EEPROM，确保程序文件不会占用Bank7

/*
address为读取地址，需要四字节对其
buff为读取缓冲区，需要四字节对其（声明是为uin32_t或者int32_t）
len为读取字节数
*/

void eeprom_read(uint16_t address,void *buff,size_t len)
{
    //去除非四字节对其的部分
    address&=(~0x3);
    len&=(~0x3);

    if(address > 1ul*HAL_NV_PAGE_CNT*HAL_FLASH_PAGE_SIZE ||
            len > 1ul*HAL_NV_PAGE_CNT*HAL_FLASH_PAGE_SIZE ||
            ((uint32_t)address)+len > 1ul*HAL_NV_PAGE_CNT*HAL_FLASH_PAGE_SIZE )
    {
        //读取地址超出范围
        return;
    }


    uint16_t page_offset_start=address%HAL_FLASH_PAGE_SIZE;
    uint16_t page_offset_stop=(address+len)%HAL_FLASH_PAGE_SIZE;

    uint16_t page_start=address/HAL_FLASH_PAGE_SIZE;
    uint16_t page_stop=(address+len)/HAL_FLASH_PAGE_SIZE;

    if(page_start==page_stop)
        HalFlashRead(page_start+HAL_NV_PAGE_BEG,page_offset_start,buff,page_offset_stop-page_offset_start);
    else
    {
        if(page_start+1==page_stop)
        {
            HalFlashRead(page_start+HAL_NV_PAGE_BEG,page_offset_start,buff,HAL_FLASH_PAGE_SIZE-page_offset_start);
            HalFlashRead(page_stop+HAL_NV_PAGE_BEG,0,buff+(HAL_FLASH_PAGE_SIZE-page_offset_start),page_offset_stop);
        }
        else
        {
            uint16_t bytenumhasread=0;
            HalFlashRead(page_start+HAL_NV_PAGE_BEG,page_offset_start,buff,HAL_FLASH_PAGE_SIZE-page_offset_start);
            bytenumhasread=HAL_FLASH_PAGE_SIZE-page_offset_start;
            page_start++;
            while(page_start!=page_stop)
            {
                HalFlashRead(page_start+HAL_NV_PAGE_BEG,0,buff+bytenumhasread,HAL_FLASH_PAGE_SIZE);
                bytenumhasread+=HAL_FLASH_PAGE_SIZE;
                page_start++;
            }
            HalFlashRead(page_stop+HAL_NV_PAGE_BEG,0,buff+(bytenumhasread),page_offset_stop);

        }
    }
}

/*
擦除address所在页(2048字节)
*/
void eeprom_erase(uint16_t address)
{
    HalFlashErase(address%HAL_FLASH_PAGE_SIZE+HAL_NV_PAGE_BEG);
}

/*
address为写入地址，需要四字节对其
buff为写取缓冲区，需要四字节对其（声明是为uin32_t或者int32_t）
len为写入字节数
*/
void eeprom_write(uint16_t address,void *buff,uint16_t len)
{
    //去除非四字节对其的部分
    address/=4;
    len/=4;

    if(address > 1ul*HAL_NV_PAGE_CNT*HAL_FLASH_PAGE_SIZE ||
            len > 1ul*HAL_NV_PAGE_CNT*HAL_FLASH_PAGE_SIZE ||
            ((uint32_t)address)+len > 1ul*HAL_NV_PAGE_CNT*HAL_FLASH_PAGE_SIZE )
    {
        //读取地址超出范围
        return;
    }
    HalFlashWrite(address+1ul*HAL_NV_PAGE_BEG*HAL_FLASH_PAGE_SIZE,buff,len);
}


