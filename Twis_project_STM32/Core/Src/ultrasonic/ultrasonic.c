/*
 * ultrasonic.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

// ultrasonic.c
 #include "ultrasonic.h"

// piny
#define TRIG_PORT GPIOB
#define TRIG_PIN  GPIO_PIN_5   // PB5

#define ECHO_PORT GPIOB
#define ECHO_PIN  GPIO_PIN_4   // PB4

static TIM_HandleTypeDef *u_htim = NULL;

static void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(u_htim, 0);
  while (__HAL_TIM_GET_COUNTER(u_htim) < us) { }
}

// wait na ECHO stav s timeoutom v us; return 1=timeout, 0=ok
static int wait_echo_state(GPIO_PinState state, uint32_t timeout_us)
{
  __HAL_TIM_SET_COUNTER(u_htim, 0);
  while (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) != state)
  {
    if (__HAL_TIM_GET_COUNTER(u_htim) > timeout_us) return 1;
  }
  return 0;
}

void Ultrasonic_Init(TIM_HandleTypeDef *htim)
{
  u_htim = htim;
  HAL_TIM_Base_Start(u_htim);   // TIM1 musí bežať
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}

float Ultrasonic_ReadDistanceM(void)
{
  uint32_t t_us;

  // TRIG 10 us
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
  delay_us(2);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
  delay_us(10);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

  // čakaj na ECHO HIGH (timeout 30ms)
  if (wait_echo_state(GPIO_PIN_SET, 30000)) return -1.0f;

  // meraj HIGH šírku (timeout 30ms)
  __HAL_TIM_SET_COUNTER(u_htim, 0);
  if (wait_echo_state(GPIO_PIN_RESET, 30000)) return -1.0f;

  t_us = __HAL_TIM_GET_COUNTER(u_htim);

  // m ≈ us / 5800
  return (float)t_us / 5800.0f;
}
