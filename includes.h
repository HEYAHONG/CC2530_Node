#ifndef INCLUDES_H_INCLUDED
#define INCLUDES_H_INCLUDED
#include "contiki.h"
#include "contiki-conf.h"


/*
由于SDCC及8051内核的特性，一般不需要多个.h文件,因此，直接可直接包含.c文件，编译也之编译node_main.c文件。
*/

//包含hal源代码
#include "haldma.c"
#include "halflash.c"


#ifdef INCLUDE_EEPROM_C
#include "eeprom.c"
#endif // INCLUDE_EEPROM_C


#endif // INCLUDES_H_INCLUDED
