#include <Arduino_ZeroDAC.h>
#include <Adafruit_ZeroDMA.h>

#include <algorithm> // std::swap

Arduino_ZeroDAC dac;
Adafruit_ZeroDMA dma;
DmacDescriptor * desc[2];
uint16_t * buffer[2];
uint16_t * buf_front, * buf_back;
const size_t block_size = 128;

void dma_callback(Adafruit_ZeroDMA *dma) {
  std::swap(buf_front, buf_back);
}

void setup() {
  dac.begin(DAC_REFSEL_AVCC);
  dac.startTimer(44100);

  uint8_t c = dac.channels;
  buffer[0] = new uint16_t[block_size * c];
  buffer[1] = new uint16_t[block_size * c];
  buf_front = buffer[0];
  buf_back = buffer[1];

  float phase = 0;
  float inc = 3.14159265 / block_size;

  uint8_t b = dac.bits;
  uint16_t z = ((1 << b) >> 1) - 1;
  uint16_t *p = buffer[0];
  for (size_t i=0; i<block_size; i++) {
    for (int j=0; j<c; j++)
      p[j] = z + sinf(phase) * z;
    phase += inc;
    p += c;
  }
  p = buffer[1];
  for (size_t i=0; i<block_size; i++) {
    for (int j=0; j<c; j++)
      p[j] = z + sinf(phase) * z;
    phase += inc;
    p += c;
  }

  dma.allocate();

  dma.setTrigger(dac.trigger);
  dma.setAction(DMA_TRIGGER_ACTON_BEAT);

  desc[0] = dma.addDescriptor(
    buffer[0],
    (void*)dac.data,
    block_size,
    (dma_beat_size)dac.beat_size,
    true,
    false);
	desc[0]->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

  desc[1] = dma.addDescriptor(
    buffer[1],
    (void*)dac.data,
    block_size,
    (dma_beat_size)dac.beat_size,
    true,
    false);
	desc[1]->BTCTRL.bit.BLOCKACT = DMA_BLOCK_ACTION_INT;

  dma.loop(true);
  dma.setCallback(dma_callback);

  dma.startJob();
}

void loop() {
}
