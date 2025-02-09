# Tarefa4: Comunicação Serial e Controle de Periféricos no RP2040

Este projeto tem como objetivo consolidar conceitos sobre interfaces de comunicação serial no  RP2040, utilizando a placa de desenvolvimento BitDogLab . A proposta envolve a manipulação de LEDs, botões, exibição de informações no display SSD1306  e comunicação via UART .


## Componentes Utilizados
| Componente        | GPIOs Utilizados |
|------------------|----------------|
| Matriz 5x5 WS2812 | 7              |
| LED RGB          | 11, 12, 13      |
| Botão A         | 5              |
| Botão B         | 6              |
| Display SSD1306  | SDA (14), SCL (15) |

## Funcionalidades do Projeto

### Modificação da Biblioteca `font.h`
- Adicionar **caracteres minúsculos** à biblioteca `font.h`.


### Entrada de Caracteres via Serial
- O **Serial Monitor** do **VS Code** será utilizado para digitação.
- Cada caractere digitado será exibido no **display SSD1306**.
- Caso um número **(0-9)** seja digitado, um **símbolo correspondente** será exibido na **matriz WS2812**.

### Interação com os Botões
#### **Botão A (GPIO 5)**
- Pressionar alterna o **LED Verde** do RGB.
- Exibe status no **display SSD1306**.
- Envia descrição para o **Serial Monitor**.

#### **Botão B (GPIO 6)**
- Pressionar alterna o **LED Azul** do RGB.
- Exibe status no **display SSD1306**.
- Envia descrição para o **Serial Monitor**.

### Requisitos Técnicos
- **Uso de Interrupções (IRQ)** para lidar com os botões.
- **Debounce via software** para evitar acionamentos falsos.
- **Controle de LEDs** comuns e **endereçáveis (WS2812)**.
- **Uso do Display SSD1306 (128x64)**, incluindo **fontes maiúsculas e minúsculas**.
- **Comunicação Serial UART** para envio e recepção de dados.
- **Código estruturado e comentado**, garantindo clareza e organização.

##Exemplo de Saída no Serial Monitor
```
Caractere recebido: A
Exibindo no display: A
Botão A pressionado: LED Verde LIGADO
Botão B pressionado: LED Azul DESLIGADO
```
