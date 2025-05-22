#ifndef ROOM_CONTROL_H
#define ROOM_CONTROL_H

#include <stdint.h>

/**
 * @brief Función a ser llamada por EXTI15_10_IRQHandler cuando se detecta
 *        la pulsación del botón B1.
 */
void room_control_on_button_press(void);

/**
 * @brief Función a ser llamada por USART2_IRQHandler cuando se recibe un carácter.
 * @param received_char El carácter recibido por UART.
 */
void room_control_on_uart_receive(char received_char);

/**
 * @brief (Opcional) Función para realizar inicializaciones específicas de la lógica
 *        de room_control, si las hubiera (ej. inicializar variables de estado).
 *        Las inicializaciones de periféricos se harán en main().
 */
void room_control_app_init(void);

/**
 * @brief Función a ser llamada periódicamente por SysTick_Handler.
 *        Aquí puedes colocar tareas que deban ejecutarse cada tick.
 */
void room_control_on_systick(void);

#endif // ROOM_CONTROL_H
#include "room_control.h"

#include "gpio.h"    // Para controlar LEDs y leer el botón (aunque el botón es por EXTI)
#include "systick.h" // Para obtener ticks y manejar retardos/tiempos
#include "uart.h"    // Para enviar mensajes
#include "tim.h"     // Para controlar el PWM
static uint32_t external_led_off_time = 0;

// Función para establecer el brillo del LED mediante PWM
void room_control_set_pwm_led(uint8_t on)
{
    // Si on es 0, apaga el LED PWM; si es distinto de 0, enciende al 80%
    if (on) {
        tim3_ch1_pwm_set_duty_cycle(80); // 80% brillo
    } else {
        tim3_ch1_pwm_set_duty_cycle(0);   // 0% brillo
    }
}

void room_control_app_init(void)
{
    tim3_ch1_pwm_set_duty_cycle(0); // Duty cycle inicial 0% para el PWM LED
}

void room_control_on_button_press(void)
{
    static uint32_t last_press_time = 0;
    uint32_t current_time = systick_get_tick();
    if (current_time - last_press_time < 200) // 200 ms de debounce
    {
        return;
    }
    last_press_time = current_time;

    // Encender LED externo en PA4
    gpio_write_pin(GPIOA, 4, GPIO_PIN_SET);
    uart2_send_string("LED externo: ON\r\n");

    // Programar apagado en 3 segundos
    external_led_off_time = current_time + 3000;
    
}

void room_control_on_uart_receive(char received_char)
{
    switch (received_char)
    {
        case 'h':
        case 'H':
            room_control_set_pwm_led(1); // Enciende el LED PWM al 80%
            uart2_send_string("LED PWM: 80%\r\n");
            break;
        case 'l':
        case 'L':
            room_control_set_pwm_led(0); // Apaga el LED PWM (0%)
            uart2_send_string("LED PWM: 0%\r\n");
            break;
        case 't':
        case 'T':
            gpio_toggle_pin(GPIOA, 4); // Cambia el estado del LED externo
            uart2_send_string("LED externo: TOGGLE\r\n");
            break;
        default:
            uart2_send_char(received_char);
            break;
        
    }
}

void room_control_on_systick(void)
{
    // Parpadeo de LD2 (PA5) cada 500 ms
    static uint32_t last_toggle = 0;
    uint32_t now = systick_get_tick();
    if (now - last_toggle >= 500) { // 500 ms
        last_toggle = now;
        gpio_toggle_pin(GPIOA, 5); // Cambia el estado de LD2
    }

    // Apagar LED externo después de 3 segundos
    if (external_led_off_time && now >= external_led_off_time) {
        gpio_write_pin(GPIOA, 4, GPIO_PIN_RESET);
        uart2_send_string("LED externo: OFF\r\n");
        external_led_off_time = 0;
    }
}