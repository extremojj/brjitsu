Este código é um exemplo de como usar o Raspberry Pi Pico para controlar uma matriz de LEDs WS2812 e um LED RGB usando botões e interrupções. O código exibe números de 0 a 9 na matriz de LEDs e permite incrementar com o botão A ou decrementar com o botão B o número exibido. Além disso, o LED vermelho do LED RGB pisca continuamente 5 vezes por segundo usando um timer.

WS2812_PIN: Pino GPIO ao qual os LEDs WS2812 estão conectados.

BUTTON_A e BUTTON_B: Pinos GPIO aos quais os botões A e B estão conectados.

LED_RED_PIN, LED_BLUE_PIN, LED_GREEN_PIN: Pinos GPIO aos quais os LEDs RGB estão conectados.

Matriz de Padrões:

A matriz numeros contém os padrões para os números de 0 a 9, representados em uma matriz 5x5.
O código faz uso de interrupções e debouncing para garantir uma interação eficiente e precisa com os botões
Funções Principais
update_led_buffer(): Atualiza o buffer de LEDs com o número selecionado.

set_leds_from_buffer(): Envia o estado do buffer para a matriz de LEDs.

gpio_callback(): Função de interrupção para tratar os pressionamentos dos botões A e B com debouncing de 200 ms.

repeating_timer_callback(): Função de callback do timer para piscar o LED vermelho do RGB a cada 100 ms (5 vezes por segundo).
