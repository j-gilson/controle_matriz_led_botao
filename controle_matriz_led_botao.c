#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

#define LED_COUNT 25  // 5x5 matriz de LEDs
#define LED_PIN 7

struct pixel_t {
  uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

npLED_t leds[LED_COUNT];
PIO np_pio;
uint sm;

const uint BUTTON_A_PIN = 5;
const uint BUTTON_B_PIN = 6;

int posX = 0;
int posY = 0;

void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true);
  }
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  npClear();
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100);
}

int getLEDIndex(int x, int y) {
  if (y % 2 == 0) {
    return y * 5 + x;
  } else {
    return y * 5 + (4 - x);
  }
}

void updateLED() {
  npClear();
  int index = getLEDIndex(posX, posY);
  npSetLED(index, 255, 255, 255);
  npWrite();
}

int main() {
  stdio_init_all();
  npInit(LED_PIN);
  npClear();
 
  gpio_init(BUTTON_A_PIN);
  gpio_init(BUTTON_B_PIN);
  gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
  gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_A_PIN);
  gpio_pull_up(BUTTON_B_PIN);

  updateLED();

  while (true) {
    if (gpio_get(BUTTON_A_PIN) == 0) {
      posX = (posX + 1) % 5;
      updateLED();
      sleep_ms(200);
    }

    if (gpio_get(BUTTON_B_PIN) == 0) {
      posY = (posY + 1) % 5;
      updateLED();
      sleep_ms(200);
    }
    sleep_ms(50);
  }
  return 0;
}
