#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// Definições para os botões
#define BUTTON_A 5
#define BUTTON_B 6

// GPIOs do LED RGB
#define LED_RED_PIN 13
#define LED_BLUE_PIN 12
#define LED_GREEN_PIN 11

// Matriz de padrões para números de 0 a 9 (5x5)
const uint32_t numeros[10][25] = {
    // Número 0
    {0, 0, 1, 0, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 0, 1, 0, 0},
    // Número 1
    {0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0},
    // Número 2
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 3
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 4
    {0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0},
    // Número 5
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0},
    // Número 6
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0},
    // Número 7
    {0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 8
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 9
    {0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0}
};

// Variáveis globais para controle do LED e cor
volatile uint8_t selected_number = 0; // Índice do número a ser exibido (0 a 9)
uint8_t selected_r = 0;   // Intensidade do vermelho (0 a 255)
uint8_t selected_g = 0;   // Intensidade do verde (0 a 255)
uint8_t selected_b = 20;  // Intensidade do azul (0 a 255)
volatile absolute_time_t last_interrupt_time = 0; // Tempo da última interrupção
volatile bool led_state = false; // Estado atual do LED vermelho

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

uint32_t led_buffer[NUM_PIXELS] = {0}; // Buffer para armazenar as cores de todos os LEDs

void update_led_buffer() {
    // Apaga todos os LEDs
    for (int i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = 0; // Desliga todos os LEDs
    }

    // Configura a cor para o número selecionado
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (numeros[selected_number][i] == 1) {
            led_buffer[i] = urgb_u32(selected_r, selected_g, selected_b);
        }
    }
}

void set_leds_from_buffer() {
    // Envia o estado de todos os LEDs para a matriz
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(led_buffer[i]);
    }
}

// Função de Interrupção com Debouncing
void gpio_callback(uint gpio, uint32_t events) {
    // Verifica o tempo desde a última interrupção
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_interrupt_time, now) < 200000) { // 200 ms de debounce. o tempo q o professor usou na aula
        return;
    }
    last_interrupt_time = now;

    if (gpio == BUTTON_A) {
        selected_number = (selected_number + 1) % 10;
    }
    if (gpio == BUTTON_B) {
        selected_number = (selected_number + 9) % 10; // Decremento circular
    }
}

// Função de Callback 
bool repeating_timer_callback(struct repeating_timer *t) {
    led_state = !led_state;
    gpio_put(LED_RED_PIN, led_state);
    return true; // Retorna true para continuar repetindo
}

int main() {
    stdio_init_all();
    printf("WS2812 5x5 Matrix - Animação numerica com botoes e interrupcoes\n");

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Inicializa os pinos dos botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Inicializa o pino do LED vermelho do RGB
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    // Configura o Timer para piscar o LED vermelho 5 vezes por segundo (a cada 100 ms)
    struct repeating_timer timer;
    add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);

    while (1) {
        // Atualiza o buffer com o número selecionado
        update_led_buffer();

        // Envia o estado do buffer para a matriz
        set_leds_from_buffer();

        sleep_ms(100); // Pequeno atraso
    }

    return 0;
}