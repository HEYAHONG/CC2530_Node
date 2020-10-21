#include "hal_adc.h"
#include "contiki.h"
#include "contiki-conf.h"
/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
#define HAL_ADC_EOC         0x80    /* End of Conversion bit */
#define HAL_ADC_START       0x40    /* Starts Conversion */

#define HAL_ADC_STSEL_EXT   0x00    /* External Trigger */
#define HAL_ADC_STSEL_FULL  0x10    /* Full Speed, No Trigger */
#define HAL_ADC_STSEL_T1C0  0x20    /* Timer1, Channel 0 Compare Event Trigger */
#define HAL_ADC_STSEL_ST    0x30    /* ADCCON1.ST =1 Trigger */

#define HAL_ADC_RAND_NORM   0x00    /* Normal Operation */
#define HAL_ADC_RAND_LFSR   0x04    /* Clock LFSR */
#define HAL_ADC_RAND_SEED   0x08    /* Seed Modulator */
#define HAL_ADC_RAND_STOP   0x0c    /* Stop Random Generator */
#define HAL_ADC_RAND_BITS   0x0c    /* Bits [3:2] */

#define HAL_ADC_DEC_064     0x00    /* Decimate by 64 : 8-bit resolution */
#define HAL_ADC_DEC_128     0x10    /* Decimate by 128 : 10-bit resolution */
#define HAL_ADC_DEC_256     0x20    /* Decimate by 256 : 12-bit resolution */
#define HAL_ADC_DEC_512     0x30    /* Decimate by 512 : 14-bit resolution */
#define HAL_ADC_DEC_BITS    0x30    /* Bits [5:4] */

#define HAL_ADC_STSEL       HAL_ADC_STSEL_ST
#define HAL_ADC_RAND_GEN    HAL_ADC_RAND_STOP
#define HAL_ADC_REF_VOLT    HAL_ADC_REF_AVDD
#define HAL_ADC_DEC_RATE    HAL_ADC_DEC_064
#define HAL_ADC_SCHN        HAL_ADC_CHN_VDD3
#define HAL_ADC_ECHN        HAL_ADC_CHN_GND


uint8 HalAdcRef=HAL_ADC_REF_VOLT;

/**************************************************************************************************
 * @fn      HalAdcRead
 *
 * @brief   Read the ADC based on given channel and resolution
 *
 * @param   channel - channel where ADC will be read
 * @param   resolution - the resolution of the value
 *
 * @return  16 bit value of the ADC in offset binary format.
 *
 *          Note that the ADC is "bipolar", which means the GND (0V) level is mid-scale.
 *          Note2: This function assumes that ADCCON3 contains the voltage reference.
 **************************************************************************************************/
uint16 HalAdcRead (uint8 channel, uint8 resolution)
{
  int16  reading = 0;
  uint8   i, resbits;
  uint8  adcChannel = 1;

  /*
   * If Analog input channel is AIN0..AIN7, make sure corresponing P0 I/O pin is enabled.  The code
   * does NOT disable the pin at the end of this function.  I think it is better to leave the pin
   * enabled because the results will be more accurate.  Because of the inherent capacitance on the
   * pin, it takes time for the voltage on the pin to charge up to its steady-state level.  If
   * HalAdcRead() has to turn on the pin for every conversion, the results may show a lower voltage
   * than actuality because the pin did not have time to fully charge.
   */
  if (channel < 8)
  {
    for (i=0; i < channel; i++)
    {
      adcChannel <<= 1;
    }
  }

  /* Enable channel */
  //ADCCFG |= adcChannel;

  /* Convert resolution to decimation rate */
  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      resbits = HAL_ADC_DEC_064;
      break;
    case HAL_ADC_RESOLUTION_10:
      resbits = HAL_ADC_DEC_128;
      break;
    case HAL_ADC_RESOLUTION_12:
      resbits = HAL_ADC_DEC_256;
      break;
    case HAL_ADC_RESOLUTION_14:
    default:
      resbits = HAL_ADC_DEC_512;
      break;
  }

  /* writing to this register starts the extra conversion */
  ADCCON3 = channel | resbits | HalAdcRef;

  /* Wait for the conversion to be done */
  while (!(ADCCON1 & HAL_ADC_EOC));

  /* Disable channel after done conversion */
  //ADCCFG &= (adcChannel ^ 0xFF);

  /* Read the result */
  reading = (int16) (ADCL);
  reading |= (int16) (ADCH << 8);

  /* Treat small negative as 0 */
  if (reading < 0)
    reading = 0;

  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      reading >>= 8;
      break;
    case HAL_ADC_RESOLUTION_10:
      reading >>= 6;
      break;
    case HAL_ADC_RESOLUTION_12:
      reading >>= 4;
      break;
    case HAL_ADC_RESOLUTION_14:
    default:
      reading >>= 2;
    break;
  }


  return ((uint16)reading);
}


/**************************************************************************************************
 * @fn      HalAdcSetReference
 *
 * @brief   Sets the reference voltage for the ADC and initializes the service
 *
 * @param   reference - the reference voltage to be used by the ADC
 *
 * @return  none
 *
 **************************************************************************************************/
void HalAdcSetReference ( uint8 reference )
{

  HalAdcRef= reference;
}
