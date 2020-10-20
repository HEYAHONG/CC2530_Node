#ifndef HAL_TYPES_H_INCLUDED
#define HAL_TYPES_H_INCLUDED

#include "stdint.h"

/* ------------------------------------------------------------------------------------------------
 *                                               Types
 * ------------------------------------------------------------------------------------------------
 */
typedef int8_t   int8;
typedef uint8_t   uint8;

typedef int16_t  int16;
typedef uint16_t  uint16;

typedef int32_t   int32;
typedef uint32_t   uint32;

typedef uint8_t   bool;

typedef uint8           halDataAlign_t;


/* ------------------------------------------------------------------------------------------------
 *                                       Memory Attributes
 * ------------------------------------------------------------------------------------------------
 */

/* ----------- IAR Compiler ----------- */
#ifdef __IAR_SYSTEMS_ICC__
#define  CODE   __code
#define  XDATA  __xdata

/* ----------- GNU Compiler ----------- */
#elif defined __KEIL__
#define  CODE   code
#define  XDATA  xdata

/* ----------- Unrecognized Compiler ----------- */
#else

#define  CODE   __code
#define  XDATA  __xdata

#endif


/* ------------------------------------------------------------------------------------------------
 *                                        Standard Defines
 * ------------------------------------------------------------------------------------------------
 */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif


#endif // HAL_TYPES_H_INCLUDED
