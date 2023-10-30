/*!
 * @file Arduino_ZeroDAC.cpp
 *
 * @mainpage Arduino DAC peripheral driver for SAMD21 and SAMD51 chips
 *
 * @section intro_sec Introduction
 *
 *  DAC peripheral driver for SAMD21 and SAMD51 chips
 *
 *
 * @section author Author
 *
 * Written by Tobias Ebsen.
 *
 * @section license License
 *
 * MIT license, all text here must be included in any redistribution.
 *
 */

#include "Arduino_ZeroDAC.h"
#include "wiring_private.h"

#if defined(__SAMD21__)
const uint8_t Arduino_ZeroDAC::channels = 1;
const uint8_t Arduino_ZeroDAC::bits = 10;
const void *Arduino_ZeroDAC::data = (const void *)(&DAC->DATA.reg);
const uint8_t Arduino_ZeroDAC::beat_size = DMAC_BTCTRL_BEATSIZE_HWORD_Val;
const uint8_t Arduino_ZeroDAC::trigger = TC5_DMAC_ID_OVF;
#elif defined(__SAMD51__)
const uint8_t Arduino_ZeroDAC::channels = 2;
const uint8_t Arduino_ZeroDAC::bits = 12;
const void *Arduino_ZeroDAC::data = (const void *)(&DAC->DATA[0].reg);
const uint8_t Arduino_ZeroDAC::beat_size = DMAC_BTCTRL_BEATSIZE_WORD_Val;
const uint8_t Arduino_ZeroDAC::trigger = TC2_DMAC_ID_OVF;
#else
const uint8_t Arduino_ZeroDAC::channels = 0;
const uint8_t Arduino_ZeroDAC::bits = 0;
const void *Arduino_ZeroDAC::data = (const void *)0;
const uint8_t Arduino_ZeroDAC::beat_size = 0;
const uint8_t Arduino_ZeroDAC::trigger = 0;
#endif

/**************************************************************************/
/*!
    @brief  start up the DAC peripheral
        @param vref voltage reference source for the dac
*/
/**************************************************************************/
void Arduino_ZeroDAC::begin(DACVRef vref) {
#if defined(__SAMD21__)

  // Power Manager: enable DAC
  PM->APBCMASK.reg |= PM_APBCMASK_DAC;

  // Control A //

  // Software Reset
  DAC->CTRLA.reg = DAC_CTRLA_SWRST;
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

  // Enable
  DAC->CTRLA.reg = DAC_CTRLA_ENABLE;
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

  // Control B //

  // Voltage reference
  uint8_t reg = DAC_CTRLB_IOEN | DAC_CTRLB_EOEN | DAC_CTRLB_BDWP;
  switch (vref) {
  case DAC_REFSEL_INT1V:
    reg |= DAC_CTRLB_REFSEL_INT1V;
    break;
  case DAC_REFSEL_AVCC:
    reg |= DAC_CTRLB_REFSEL_AVCC;
    break;
  case DAC_REFSEL_VREF:
    reg |= DAC_CTRLB_REFSEL_VREFP;
    break;
  }
  DAC->CTRLB.reg = reg;
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

  // Event Control
  DAC->EVCTRL.reg = DAC_EVCTRL_STARTEI | DAC_EVCTRL_EMPTYEO;
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

  // Setup interrupts
  DAC->INTFLAG.reg = DAC_INTFLAG_EMPTY; // clear pending flags
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

  DAC->INTENSET.reg = DAC_INTENSET_EMPTY;
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

  // Enable
  DAC->CTRLA.reg = DAC_CTRLA_ENABLE;
  while (DAC->STATUS.bit.SYNCBUSY)
    ;

#endif
#if defined(__SAMD51__)

  pinPeripheral(PIN_DAC0, PIO_ANALOG);
  pinPeripheral(PIN_DAC1, PIO_ANALOG);

  // Control A //

  // Software Reset
  DAC->CTRLA.reg = DAC_CTRLA_SWRST;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  // Disable
  DAC->CTRLA.bit.ENABLE = 0;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  // Control B //

  // Voltage reference
  uint8_t reg = 0;
  switch (vref) {
  case DAC_REFSEL_INT1V:
    reg |= DAC_CTRLB_REFSEL_INTREF;
    break;
  case DAC_REFSEL_AVCC:
    reg |= DAC_CTRLB_REFSEL_VDDANA;
    break;
  case DAC_REFSEL_VREF:
    reg |= DAC_CTRLB_REFSEL_VREFPU;
    break;
  }
  DAC->CTRLB.reg = reg;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  // Event Control
  DAC->EVCTRL.reg = 0;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  // Setup interrupts
  DAC->INTFLAG.reg =
      DAC_INTFLAG_EMPTY0 | DAC_INTFLAG_EMPTY1; // clear pending flags
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  DAC->INTENSET.reg = DAC_INTENSET_EMPTY0 | DAC_INTENSET_EMPTY1;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  // Enable
  DAC->DACCTRL[0].bit.ENABLE = 1;
  DAC->DACCTRL[1].bit.ENABLE = 1;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

  // Enable
  DAC->CTRLA.reg |= DAC_CTRLA_ENABLE;
  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;

#endif
}

void Arduino_ZeroDAC::write(uint16_t data) {
#if defined(__SAMD21__)
  while (DAC->STATUS.bit.SYNCBUSY)
    ;
  DAC->DATA.reg = data;
#endif
#if defined(__SAMD51__)

  while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST)
    ;
  DAC->DATA[0].reg = data;
#endif
}

void Arduino_ZeroDAC::startTimer(uint16_t sample_rate) {
#if defined(__SAMD21__)

  // Generic Clock
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 |
                                 GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;

  // Power Manager: enable Timer/Counter
  PM->APBCMASK.reg |= PM_APBCMASK_TC5;

  // Software Reset
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY)
    ;
  while (TC5->COUNT16.CTRLA.bit.SWRST)
    ;

  // Disable
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY)
    ;

  // Configure Control A
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 |  // 16-bit counter mode
                           TC_CTRLA_WAVEGEN_MFRQ |  // Match Frequency mode
                           TC_CTRLA_PRESCALER_DIV1; // 1:1 Prescale
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY)
    ;

  TC5->COUNT16.CTRLBCLR.reg = TC_CTRLBCLR_MASK;

  // Set counter compare value
  TC5->COUNT16.CC[0].reg = (uint16_t)(SystemCoreClock / sample_rate);
  while (TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY)
    ;

  // Enable
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY)
    ;

#endif
#if defined(__SAMD51__)

  // Generic Clock
  // while ((GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL2));
  GCLK->PCHCTRL[TC2_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK2;
  // while (!(GCLK->PCHCTRL[TC2_GCLK_ID].reg & GCLK_PCHCTRL_CHEN));

  TC2->COUNT8.WAVE.reg = TC_WAVE_WAVEGEN_NFRQ;

  // Disable
  TC2->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST)
    ;

  // Software Reset
  TC2->COUNT8.CTRLA.reg = TC_CTRLA_SWRST;
  while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST)
    ;
  while (TC2->COUNT8.CTRLA.bit.SWRST)
    ;

  // Mode and prescaler
  TC2->COUNT8.CTRLA.reg |= TC_CTRLA_MODE_COUNT8 | TC_CTRLA_PRESCALER_DIV16;
  while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST)
    ;

  // Period
  TC2->COUNT8.PER.reg = (uint8_t)((VARIANT_GCLK2_FREQ >> 4) / sample_rate);
  while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST)
    ;

  // Enable
  TC2->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST)
    ;

#endif
}
