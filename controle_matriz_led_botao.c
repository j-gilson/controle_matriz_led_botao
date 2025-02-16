#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

#define LED_COUNT 25  // Definição do número total de LEDs (matriz 5x5)
#define LED_PIN 7     // Pino ao qual os LEDs estão conectados

// Estrutura para representar um pixel RGB (cada LED)
struct pixel_t {
  uint8_t G, R, B;  // Ordem dos componentes de cor no protocolo WS2812B
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;  // Alias para facilitar a leitura

// Array para armazenar o estado dos LEDs da matriz
npLED_t leds[LED_COUNT];

// Variáveis para controle do periférico PIO
PIO np_pio;
uint sm;

// Definição dos pinos dos botões
const uint BUTTON_A_PIN = 5;
const uint BUTTON_B_PIN = 6;

// Variáveis para posição do LED ativo na matriz
int posX = 0;
int posY = 0;

/**
 * Inicializa o periférico PIO para controlar os LEDs WS2812B.
 * @param pin Pino ao qual os LEDs estão conectados.
 */
void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);  // Carrega programa PIO
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, false);  // Tenta alocar uma máquina de estado livre
  if (sm < 0) {  
    np_pio = pio1;  // Se não conseguir, usa a segunda PIO
    sm = pio_claim_unused_sm(np_pio, true);
  }
  // Inicializa o programa PIO com o clock apropriado
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  npClear();  // Limpa os LEDs ao iniciar
}

/**
 * Define a cor de um LED específico.
 * @param index Índice do LED na matriz.
 * @param r Componente vermelho (0-255).
 * @param g Componente verde (0-255).
 * @param b Componente azul (0-255).
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Apaga todos os LEDs da matriz.
 */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);  // Define todos os LEDs como apagados
}

/**
 * Escreve os valores dos LEDs na fita usando a PIO.
 */
void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100);  // Pausa para garantir estabilidade no sinal
}

/**
 * Calcula o índice de um LED na matriz 5x5 a partir das coordenadas (x, y).
 * @param x Posição na horizontal (0-4).
 * @param y Posição na vertical (0-4).
 * @return Índice correspondente no array de LEDs.
 */
int getLEDIndex(int x, int y) {
  if (y % 2 == 0) {  
    return y * 5 + x;  // Se a linha for par, mantém a ordem normal
  } else {
    return y * 5 + (4 - x);  // Se a linha for ímpar, inverte a ordem (zigue-zague)
  }
}

/**
 * Atualiza o LED ativo na matriz com cor branca.
 */
void updateLED() {
  npClear();  // Apaga todos os LEDs
  int index = getLEDIndex(posX, posY);  // Obtém índice do LED ativo
  npSetLED(index, 255, 255, 255);  // Define o LED ativo como branco
  npWrite();  // Envia os dados para os LEDs
}

/**
 * Função principal do programa.
 */
int main() {
  stdio_init_all();  // Inicializa entrada e saída padrão
  npInit(LED_PIN);  // Inicializa os LEDs
  npClear();  // Garante que os LEDs começam apagados
 
  // Configura os pinos dos botões
  gpio_init(BUTTON_A_PIN);
  gpio_init(BUTTON_B_PIN);
  gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
  gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_A_PIN);  // Habilita pull-up para leitura estável
  gpio_pull_up(BUTTON_B_PIN);

  updateLED();  // Mostra o LED inicial na posição (0,0)

  // Loop principal
  while (true) {
    if (gpio_get(BUTTON_A_PIN) == 0) {  // Se o botão A for pressionado
      posX = (posX + 1) % 5;  // Move para a direita (com loop circular)
      updateLED();  // Atualiza a matriz
      sleep_ms(200);  // Pequeno atraso para evitar múltiplos cliques
    }

    if (gpio_get(BUTTON_B_PIN) == 0) {  // Se o botão B for pressionado
      posY = (posY + 1) % 5;  // Move para baixo (com loop circular)
      updateLED();  // Atualiza a matriz
      sleep_ms(200);  // Pequeno atraso para evitar múltiplos cliques
    }

    sleep_ms(50);  // Pequeno atraso para estabilidade da leitura dos botões
  }
  return 0;  // Nunca chega aqui, pois o loop é infinito
}
