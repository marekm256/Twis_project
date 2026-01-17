/*
 * motor_control.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#include "motor_control.h"
#include "main.h"
#include "tim.h"

static uint32_t tim_clk_hz(void)
{
    RCC_ClkInitTypeDef clk = {0};
    uint32_t lat = 0;
    HAL_RCC_GetClockConfig(&clk, &lat);

    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    return (clk.APB1CLKDivider == RCC_HCLK_DIV1) ? pclk1 : (2u * pclk1);
}

static void compute_psc_arr(uint32_t pwm_hz, uint32_t *psc, uint32_t *arr)
{
    uint32_t clk = tim_clk_hz();

    if (pwm_hz < 1u) pwm_hz = 1u;

    for (uint32_t p = 0; p <= 0xFFFFu; p++)
    {
        uint32_t denom = (p + 1u) * pwm_hz;
        if (denom == 0u) continue;

        uint32_t a = (clk / denom);
        if (a == 0u) continue;
        a -= 1u;

        if (a <= 0xFFFFu) {
            *psc = p;
            *arr = a;
            return;
        }
    }

    *psc = 0xFFFFu;
    *arr = 0xFFFFu;
}

static void apply_freq_and_50pct(uint32_t freq_hz)
{
    uint32_t psc, arr;
    compute_psc_arr(freq_hz, &psc, &arr);

    __HAL_TIM_SET_PRESCALER(&htim3, psc);
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);

    uint32_t ccr = (arr + 1u) / 2u; /* 50 % */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccr);

    __HAL_TIM_SET_COUNTER(&htim3, 0u);
    HAL_TIM_GenerateEvent(&htim3, TIM_EVENTSOURCE_UPDATE);
}

static bool pwm_running = false;

static uint32_t cur_freq = 0u;
static uint32_t tgt_freq = 0u;

static uint32_t last_step_tick = 0u;
static uint32_t step_period_ms = 10u;  /* update každých 10 ms */
static uint32_t step_hz = 1u;          /* prepočíta sa podľa ramp_ms */

void Motors_Init(void)
{
    /* istota: vypnuté */
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    pwm_running = false;

    cur_freq = 0u;
    tgt_freq = 0u;
    last_step_tick = HAL_GetTick();
}

static void pwm_start_if_needed(uint32_t start_freq_hz)
{
    if (!pwm_running)
    {
        apply_freq_and_50pct(start_freq_hz);

        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

        pwm_running = true;
    }
}

static void pwm_stop_if_running(void)
{
    if (pwm_running)
    {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
        pwm_running = false;
    }
}

void Motors_Update(bool enable, uint32_t target_freq_hz, uint32_t ramp_ms)
{
    tgt_freq = enable ? target_freq_hz : 0u;

    /* ramp_ms = 0 -> skok */
    if (ramp_ms == 0u)
    {
        cur_freq = tgt_freq;

        if (cur_freq == 0u) {
            pwm_stop_if_running();
        } else {
            pwm_start_if_needed(cur_freq);
            apply_freq_and_50pct(cur_freq);
        }
        return;
    }

    /* prepočet kroku rampy (Hz per step) */
    if (target_freq_hz < 1u) target_freq_hz = 1u;

    uint32_t steps = ramp_ms / step_period_ms;
    if (steps < 1u) steps = 1u;

    step_hz = (enable ? target_freq_hz : cur_freq);
    step_hz = (step_hz + steps - 1u) / steps; /* ceil */
    if (step_hz < 1u) step_hz = 1u;

    uint32_t now = HAL_GetTick();
    if ((now - last_step_tick) < step_period_ms) return;
    last_step_tick = now;

    /* rampovanie */
    if (cur_freq < tgt_freq)
    {
        if (cur_freq == 0u) {
            /* pri štarte sa najprv zapne PWM na 1 Hz, aby timer bežal */
            pwm_start_if_needed(1u);
            cur_freq = 1u;
            apply_freq_and_50pct(cur_freq);
            return;
        }

        uint32_t next = cur_freq + step_hz;
        cur_freq = (next > tgt_freq) ? tgt_freq : next;
        apply_freq_and_50pct(cur_freq);
    }
    else if (cur_freq > tgt_freq)
    {
        if (cur_freq <= step_hz) cur_freq = 0u;
        else cur_freq -= step_hz;

        if (cur_freq == 0u) {
            pwm_stop_if_running();
        } else {
            apply_freq_and_50pct(cur_freq);
        }
    }
    else
    {
        /* drží cieľ */
        if (cur_freq == 0u) pwm_stop_if_running();
    }
}
