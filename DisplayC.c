#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define NUM_PIXELS 25
#define WS2812_PIN 7
#define IS_RGBW false

#define BUTTON_A 5
#define BUTTON_B 6
#define RED_LED_PIN 13  
#define GREEN_LED_PIN 12
#define BLUE_LED_PIN 11
#define DEBOUNCE_DELAY_MS 200  

PIO pio=pio0;
uint sm=0;


volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_button_b_time = 0;

volatile bool button_a_pressed = false;
volatile bool button_b_pressed = false;

bool led_green_state = false;
bool led_blue_state = false;

// Função para configurar um pixel da matriz de LEDs
void set_pixel(uint index, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

// Função para inverter a matriz 5x5 em 180 graus
void invert_matrix_180(uint8_t *number) {
    uint8_t inverted[NUM_PIXELS];

    // Inverter as linhas
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            inverted[(4 - row) * 5 + col] = number[row * 5 + col];
        }
    }

    // Copiar a matriz invertida de volta
    for (int i = 0; i < NUM_PIXELS; i++) {
        number[i] = inverted[i];
    }
}

// Mapeamento dos números de 0 a 9 para a matriz 5x5
const uint8_t numbers[10][NUM_PIXELS] = {
    // Número 0
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 1
    {0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 1, 0},
    // Número 2
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0},
    // Número 3
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 4
    {0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0},
    // Número 5
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 6
    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 7
    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0},
    // Número 8
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0},
    // Número 9
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 0, 0, 0}
};


// Função para exibir um número na matriz de LEDs com brilho reduzido
void display_number(int number) {
    uint8_t temp[NUM_PIXELS];

    // Copiar o número para uma variável temporária para inverter
    for (int i = 0; i < NUM_PIXELS; i++) {
        temp[i] = numbers[number][i];
    }

    invert_matrix_180(temp);  // Inverte a matriz 180 graus

    // Fator de atenuação (controle de brilho)
    float brightness_factor = 0.2;  // 20% de brilho

    // Exibir os LEDs da matriz com brilho ajustado
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int index = row * 5 + col;
            if (temp[index] == 1) {
                // Aplicando a atenuação na cor azul
                uint32_t green_color = 0xFF0000;
                uint8_t r = (green_color >> 16) & 0xFF;
                uint8_t g = (green_color >> 8) & 0xFF;
                uint8_t b = green_color & 0xFF;

                // Aplicando o fator de brilho
                r = (uint8_t)(r * brightness_factor);
                g = (uint8_t)(g * brightness_factor);
                b = (uint8_t)(b * brightness_factor);

                // Recria a cor ajustada e define o pixel
                uint32_t dimmed_color = (r << 16) | (g << 8) | b;
                set_pixel(index, dimmed_color);
            } else {
                set_pixel(index, 0x000000);  // Apaga o LED
            }
        }
    }
}

void set_rgb_led(bool r, bool g, bool b) {
    gpio_put(RED_LED_PIN, r);
    gpio_put(GREEN_LED_PIN, g);
    gpio_put(BLUE_LED_PIN, b);
}


void button_isr(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A && (current_time - last_button_a_time > DEBOUNCE_DELAY_MS)) {
        last_button_a_time = current_time;
        led_green_state = !led_green_state;
        set_rgb_led(false, false ,led_green_state);  
        button_a_pressed = true;
    } 
    else if (gpio == BUTTON_B && (current_time - last_button_b_time > DEBOUNCE_DELAY_MS)) {
        last_button_b_time = current_time;
        led_blue_state = !led_blue_state;
        set_rgb_led(false, led_blue_state, false);  
        button_b_pressed = true;
    }
}

int main()
{
    // Inicialização do I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Função do pino GPIO como I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Função do pino GPIO como I2C
    gpio_pull_up(I2C_SDA); // Puxar a linha de dados para cima
    gpio_pull_up(I2C_SCL); // Puxar a linha de clock para cima
    
    // Inicialização do display SSD1306
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa a comunicação serial
    stdio_init_all();

    sleep_ms(2000);

    // Configuração dos LEDs RGB
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    // Configuração dos botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Configura as interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_isr);

    // Inicializar a configuração do PIO
    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), WS2812_PIN, 800000, IS_RGBW);

    // Variável para armazenar o caractere recebido
    char recebido;

    while (true) {
        if (button_a_pressed) {
            button_a_pressed = false;  // Reseta a flag
    
            // Mensagem no Serial Monitor e atualização do display
            if (led_green_state) {
                printf("Botão A pressionado: LED Verde LIGADO\n");
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "LED Verde ON", 10, 30);
                ssd1306_send_data(&ssd);
            } else {
                printf("Botão A pressionado: LED Verde DESLIGADO\n");
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "LED Verde OFF", 10, 30);
                ssd1306_send_data(&ssd);
            }
        }
    
        if (button_b_pressed) {
            button_b_pressed = false;  // Reseta a flag
    
            if (led_blue_state) {
                printf("Botão B pressionado: LED Azul LIGADO\n");
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "LED Azul ON", 10, 30);
                ssd1306_send_data(&ssd);
            } else {
                printf("Botão B pressionado: LED Azul DESLIGADO\n");
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "LED Azul OFF", 10, 30);
                ssd1306_send_data(&ssd);
            }
        }
    
        // Lê o caractere recebido, com timeout de 1 segundo
        int caracter_recebido = getchar_timeout_us(1000000);
    
        if (caracter_recebido != PICO_ERROR_TIMEOUT) {
            char recebido = (char)caracter_recebido;
            ssd1306_fill(&ssd, false);
            char string[2] = {recebido, '\0'};
            ssd1306_draw_string(&ssd, string, 10, 30);
            ssd1306_send_data(&ssd);
    
            if (recebido >= '0' && recebido <= '9') {
                display_number(recebido - '0');
            }
        }
    // Pequeno delay para evitar sobrecarga
    sleep_ms(100);
    }    

}
