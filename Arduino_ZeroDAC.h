
/*!
 * @file Arduino_ZeroDAC.h
 *
 * This is a library for the DAC peripheral on SAMD21 and SAMD51 devices
 *
 * Written by Tobias Ebsen.
 *
 * MIT license, all text here must be included in any redistribution.
 *
 */

#ifndef ARDUINO_ZERODAC_H
#define ARDUINO_ZERODAC_H

#include <Arduino.h>

/**************************************************************************/
/*!
    @brief  voltage reference
*/
/**************************************************************************/
typedef enum _DACVRef {
  DAC_REFSEL_INT1V, // Internal 1V reference
  DAC_REFSEL_AVCC,  // Analog supply reference
  DAC_REFSEL_VREF   // External reference
} DACVRef;

/**************************************************************************/
/*!
    @brief  Class that stores state and functions for interacting with DAC
   peripheral on SAMD21 and SAMD51 devices
*/
/**************************************************************************/
class Arduino_ZeroDAC {
public:
  void begin(DACVRef vref);

  void write(uint16_t data);
  void write(uint16_t data0, uint16_t data1);

  void startTimer(uint16_t sample_rate);
  void setTimer(uint16_t sample_rate);
  void stopTimer();

  static const uint8_t channels;
  static const uint8_t bits;
  static const void *data;
  static const uint8_t trigger;
  static const uint8_t beat_size;
};

#endif
