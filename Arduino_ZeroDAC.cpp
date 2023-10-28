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
    while (DAC->STATUS.bit.SYNCBUSY);

    // Enable
    DAC->CTRLA.reg = DAC_CTRLA_ENABLE;
    while (DAC->STATUS.bit.SYNCBUSY);

    // Control B //
 
    // Voltage reference
    uint8_t reg = DAC_CTRLB_IOEN | DAC_CTRLB_EOEN | DAC_CTRLB_BDWP;
    switch (vref) {
        case DAC_REFSEL_INT1V:
            reg |= DAC_CTRLB_REFSEL_INT1V; break;
        case DAC_REFSEL_AVCC:
            reg |= DAC_CTRLB_REFSEL_AVCC; break;
        case DAC_REFSEL_VREF:
            reg |= DAC_CTRLB_REFSEL_VREFP; break;
    }
    DAC->CTRLB.reg = reg;
    while (DAC->STATUS.bit.SYNCBUSY);


    // Event Control
    DAC->EVCTRL.reg = DAC_EVCTRL_STARTEI | DAC_EVCTRL_EMPTYEO;
    while (DAC->STATUS.bit.SYNCBUSY);

    // Setup interrupts
    DAC->INTFLAG.reg = DAC_INTFLAG_EMPTY; // clear pending flags
    while (DAC->STATUS.bit.SYNCBUSY);

    DAC->INTENSET.reg = DAC_INTENSET_EMPTY;
    while (DAC->STATUS.bit.SYNCBUSY);

    // Enable
    DAC->CTRLA.reg = DAC_CTRLA_ENABLE;
    while (DAC->STATUS.bit.SYNCBUSY);

#elif defined(__SAMD51__)

    // Control A //

    // Software Reset
    DAC->CTRLA.reg = DAC_CTRLA_SWRST;
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    // Enable
    DAC->CTRLA.reg = DAC_CTRLA_ENABLE;
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    // Control B //
 
    // Voltage reference
    uint8_t reg = 0;
    switch (vref) {
        case DAC_REFSEL_INT1V:
            reg |= DAC_CTRLB_REFSEL_INTREF; break;
        case DAC_REFSEL_AVCC:
            reg |= DAC_CTRLB_REFSEL_VDDANA; break;
        case DAC_REFSEL_VREF:
            reg |= DAC_CTRLB_REFSEL_VREFPU; break;
    }
    DAC->CTRLB.reg = reg;
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    // Event Control
    DAC->EVCTRL.reg = DAC_EVCTRL_STARTEI0 | DAC_EVCTRL_STARTEI1 | DAC_EVCTRL_EMPTYEO0 | DAC_EVCTRL_EMPTYEO1;
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    // Setup interrupts
    DAC->INTFLAG.reg = DAC_INTFLAG_EMPTY0 | DAC_INTFLAG_EMPTY1; // clear pending flags
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    DAC->INTENSET.reg = DAC_INTENSET_EMPTY0 | DAC_INTENSET_EMPTY1;
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    // Enable
    DAC->DACCTRL[0].bit.ENABLE = 1;
	DAC->DACCTRL[1].bit.ENABLE = 1;	
	while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

    // Enable
    DAC->CTRLA.reg = DAC_CTRLA_ENABLE;
    while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);

#endif
}

void Arduino_ZeroDAC::write(uint16_t data) {
#if defined(__SAMD21__)
	while (DAC->STATUS.bit.SYNCBUSY);
	DAC->DATA.reg = data;
#elif defined(__SAMD51__)
	while (DAC->SYNCBUSY.bit.ENABLE || DAC->SYNCBUSY.bit.SWRST);
	DAC->DATA[0].reg = data;
#endif
}

uint8_t Arduino_ZeroDAC::getChannels() {
#if defined(__SAMD21__)
	return 1;
#elif defined(__SAMD51__)
	return 2;
#else
    return 0;
#endif
}

uint8_t Arduino_ZeroDAC::getBits() {
#if defined(__SAMD21__)
	return 10;
#elif defined(__SAMD51__)
	return 12;
#else
    return 0;
#endif
}

void* Arduino_ZeroDAC::getDataRegister() {
#if defined(__SAMD21__)
	return (void*)&DAC->DATA.reg;
#elif defined(__SAMD51__)
	return (void*)&DAC->DATA[0].reg;
#else
    return 0;
#endif
}

uint8_t Arduino_ZeroDAC::getDmaBeatSize() {
#if defined(__SAMD21__)
	return DMAC_BTCTRL_BEATSIZE_HWORD_Val;
#elif defined(__SAMD51__)
	return DMAC_BTCTRL_BEATSIZE_WORD_Val;
#else
    return 0;
#endif
}

void Arduino_ZeroDAC::startTimer(uint16_t sample_rate) {
#if defined(__SAMD21__)

    // Generic Clock
    GCLK->CLKCTRL.reg =
        (uint16_t)(GCLK_CLKCTRL_CLKEN |
        GCLK_CLKCTRL_GEN_GCLK0 |
        GCLK_CLKCTRL_ID(GCM_TC4_TC5));
    while(GCLK->STATUS.bit.SYNCBUSY);

    // Power Manager: enable Timer/Counter
    PM->APBCMASK.reg |= PM_APBCMASK_TC5;

    // Software Reset
    TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
    while (TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY);
    while (TC5->COUNT16.CTRLA.bit.SWRST);

    // Disable
    TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while(TC5->COUNT16.STATUS.bit.SYNCBUSY);

    // Configure Control A
    TC5->COUNT16.CTRLA.reg =
        TC_CTRLA_MODE_COUNT16 |     // 16-bit counter mode
        TC_CTRLA_WAVEGEN_MFRQ |     // Match Frequency mode
        TC_CTRLA_PRESCALER_DIV1;    // 1:1 Prescale
    while(TC5->COUNT16.STATUS.bit.SYNCBUSY);

    TC5->COUNT16.CTRLBCLR.reg = TC_CTRLBCLR_MASK;

    // Set counter compare value
    TC5->COUNT16.CC[0].reg = (uint16_t)(SystemCoreClock / sample_rate);
    while (TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY);

    // Enable
    TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while(TC5->COUNT16.STATUS.bit.SYNCBUSY);

#elif defined(__SAMD51__)

    // Generic Clock
    GCLK->PCHCTRL[TC2_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK2;
    while (!(GCLK->PCHCTRL[TC2_GCLK_ID].reg & GCLK_PCHCTRL_CHEN));

    TC2->COUNT8.WAVE.reg = TC_WAVE_WAVEGEN_NFRQ;

    // Disable
    TC2->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST);

    // Software Reset
    TC2->COUNT8.CTRLA.reg = TC_CTRLA_SWRST;
    while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST);
    while (TC2->COUNT8.CTRLA.bit.SWRST);

    // Mode and prescaler
    TC2->COUNT8.CTRLA.reg |= TC_CTRLA_MODE_COUNT8 | TC_CTRLA_PRESCALER_DIV16;
    while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST);

    // Period
    TC2->COUNT8.PER.reg = (uint8_t)( (VARIANT_GCLK2_FREQ >> 4) / sample_rate);
    while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST);

    // Enable
    TC2->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC2->COUNT8.SYNCBUSY.bit.ENABLE || TC2->COUNT8.SYNCBUSY.bit.SWRST);

#endif
}

uint8_t Arduino_ZeroDAC::getTimerTrigger() {
#if defined(__SAMD21__)
	return TC5_DMAC_ID_OVF;
#elif defined(__SAMD51__)
	return TC2_DMAC_ID_OVF;
#else
    return 0;
#endif
}
