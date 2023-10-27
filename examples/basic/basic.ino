#include <Arduino_ZeroDAC.h>

Arduino_ZeroDAC dac;

void setup() {
  dac.begin(DAC_REFSEL_INT1V);
}

void loop() {
  dac.write(123);
}
