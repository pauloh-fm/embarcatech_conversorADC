# Tarefa 1 -Conversor adc

**Autor:** Paulo Henrique de Farias Martins

## Funcionalidades

✅ **Controle de LEDs RGB (PWM) via Joystick**  

- O **LED Azul** ajusta seu brilho conforme o **eixo Y** do joystick.  
- O **LED Vermelho** ajusta seu brilho conforme o **eixo X** do joystick.  
- O brilho **aumenta** quando o joystick é movido para os **extremos** e **diminui** ao retornar ao centro.  

✅ **Movimentação de um Quadrado no Display OLED**  

- Um **quadrado de 8x8 pixels** é exibido no **display OLED SSD1306**.  
- Ele se move conforme os valores dos **eixos X e Y** do joystick.  

✅ **Interação com os Botões**  

- **Botão do Joystick (GPIO 22):**  
  - Alterna o **LED Verde** ligado/desligado.  
  - Alterna o **estilo da borda** do display OLED.  
- **Botão A (GPIO 5):**  
  - Liga/desliga os **LEDs PWM (Vermelho e Azul)**.  

✅ **Implementação de Interrupções (IRQ) e Debounce**  

- O código utiliza **interrupções** para os botões, evitando travamentos.  
- Foi implementado **debounce por software** para evitar múltiplos acionamentos errados.  

## Instruções

Siga estes 3 passos simples para executar o projeto:

1. **Clonar o projeto:**  
   Clone este repositório para sua máquina local.

2. **Importar o projeto:**  
   Importe o projeto utilizando a extensão *Raspberry Pico Pi* no seu ambiente de desenvolvimento.

3. **Executar o projeto:**  
   Execute o projeto utilizando a extensão *Raspberry Pico Pi*.

## Demonstração de funcionamento


Assista à demonstração no YouTube:  


