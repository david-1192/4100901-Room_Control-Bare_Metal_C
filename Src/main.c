/**
 ******************************************************************************
 * @file           : main.c
 * @author         : sam C
 * @brief          : Main program body
 ******************************************************************************
 */
#include "gpio.h"
#include "systick.h"
#include "nvic.h"
#include "uart.h"
#include "tim.h"
#include "room_control.h"

int main(void)
{
    // Inicialización de SysTick
    systick_init_1ms(); // Utiliza SYSCLK_FREQ_HZ (ej. 4MHz) de rcc.h

    // LED Heartbeat (LD2) 
    gpio_setup_pin(GPIOA, 5, GPIO_MODE_OUTPUT, 0); 

    // LED Externo ON/OFF
    gpio_setup_pin(GPIOA, 4, GPIO_MODE_OUTPUT, 0);

    // Botón B1
    gpio_setup_pin(GPIOC, 13, GPIO_MODE_INPUT, 0);
    nvic_exti_pc13_button_enable();

    // USART2
    uart2_init(115200);
    nvic_usart2_irq_enable();

    // TIM3 Canal 1 para PWM
    tim3_ch1_pwm_init(1000); // ej. 1000 Hz
    tim3_ch1_pwm_set_duty_cycle(0); // ej. 0%

    // Inicialización de la Lógica de la Aplicación (room_control)
    room_control_app_init();

    // Mensaje de bienvenida o estado inicial (puede estar en room_control_app_init o aquí)
    uart2_send_string("\r\nSistema Inicializado. Esperando eventos...\r\n");

    // Para encender el LED PWM:
    room_control_set_pwm_led(1);

    // Para apagar el LED PWM:
    room_control_set_pwm_led(0);

    while (1) {
        
    }
}

