/*
 * motor_control.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#include "motor_control.h"
<<<<<<< HEAD
=======
#include "main.h"   // kvôli TIM_HandleTypeDef, makrám a externom
>>>>>>> f0626c1e1b0ce432afd47d0ec7be18ae41c45531
#include <stdint.h>

/* TIM2 handle je v main.c */
extern TIM_HandleTypeDef htim2;

/* ========================================================= */
/* Automatický výpočet PSC + ARR zo zadanej frekvencie       */
/* ========================================================= */
static void Motor_ComputeTimer(uint32_t pwm_freq,
                               uint32_t *psc,
                               uint32_t *arr)
{
    uint32_t tim_clk = HAL_RCC_GetPCLK1Freq();  // TIM2 je na APB1
    uint32_t best_arr = 0;
    uint32_t best_psc = 0;

    for (uint32_t p = 0; p <= 0xFFFF; p++)
    {
        uint32_t tmp_arr = (tim_clk / ((p + 1u) * pwm_freq)) - 1u;

        if (tmp_arr <= 0xFFFF)
        {
            best_psc = p;
            best_arr = tmp_arr;
            break;   // prvý platný → najväčší ARR
        }
    }

    *psc = best_psc;
    *arr = best_arr;
}

/* jednoduchý test */
#define MOTOR_DUTY_START   20u   // %
#define MOTOR_DUTY_MAX     50u   // %
#define MOTOR_STEP         5u    // %
#define MOTOR_STEP_MS      500u  // ms

static uint8_t duty = MOTOR_DUTY_START;

static void Motor_SetDutyPercent(uint8_t pct)
{
    if (pct > 100u) pct = 100u;

    /* ARR je v registri, keďže CubeMX nastaví period */
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim2);
    uint32_t ccr = (arr * (uint32_t)pct) / 100u;

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr);
}

void Motor_Init(uint32_t pwm_freq_hz)
{
    uint32_t psc, arr;

    Motor_ComputeTimer(pwm_freq_hz, &psc, &arr);

    /* stop PWM ak už bežal */
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

    __HAL_TIM_SET_PRESCALER(&htim2, psc);
    __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

    /* update registre */
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

void Motor_Task(void)
{
    static uint32_t last = 0;
    uint32_t now = HAL_GetTick();

    if ((now - last) >= MOTOR_STEP_MS)
    {
        last = now;

        if (duty + MOTOR_STEP <= MOTOR_DUTY_MAX) duty += MOTOR_STEP;
        else duty = MOTOR_DUTY_START;

        Motor_SetDutyPercent(duty);
    }
}


