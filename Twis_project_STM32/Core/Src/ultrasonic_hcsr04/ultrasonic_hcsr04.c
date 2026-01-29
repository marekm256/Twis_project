/*
 * ultrasonic_hcsr04.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

//
#include "ultrasonic_hcsr04.h"

/* TRIG = PA12 */
#define TRIG_PORT GPIOA
#define TRIG_PIN  GPIO_PIN_12

/* ECHO = PA8 */
#define ECHO_PORT GPIOA
#define ECHO_PIN  GPIO_PIN_8

static TIM_HandleTypeDef *u_htim = NULL;

/* mikrosekundový delay – TIM1 musí bežať na 1 MHz */
static void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(u_htim, 0);
    while (__HAL_TIM_GET_COUNTER(u_htim) < us) { }
}

/* čakaj na stav ECHO s timeoutom (µs)
   return 1 = timeout, 0 = OK
*/
static int wait_echo(GPIO_PinState state, uint32_t timeout_us)
{
    __HAL_TIM_SET_COUNTER(u_htim, 0);
    while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) != state)
    {
        if (__HAL_TIM_GET_COUNTER(u_htim) >= timeout_us)
            return 1;
    }
    return 0;
}

void Ultrasonic_Init(TIM_HandleTypeDef *htim)
{
    u_htim = htim;

    HAL_TIM_Base_Start(u_htim);

    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}

float Ultrasonic_ReadDistanceCM(void)
{
    uint32_t t_us;

    /* TRIG impulz 10 µs */
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    /* čakaj na ECHO HIGH (max 30 ms) */
    if (wait_echo(GPIO_PIN_SET, 30000))
        return -1.0f;

    /* meraj dĺžku HIGH impulzu */
    __HAL_TIM_SET_COUNTER(u_htim, 0);

    /* čakaj na ECHO LOW (max 30 ms) */
    if (wait_echo(GPIO_PIN_RESET, 30000))
        return -1.0f;

    t_us = __HAL_TIM_GET_COUNTER(u_htim);

    /* prepočet na centimetre: t_us / 5.82 */
    return (float)t_us / 5.82f;
}
