/*
 * motor_control.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#include "motors_24h055m020.h"

#include "main.h"
#include "tim.h"

#include "../comm_rpizero2w/comm_rpizero2w.h"

/* TIM2 CH1 = Left motor, TIM3 CH1 = Right motor */
#define LEFT_TIM     htim2
#define LEFT_CH      TIM_CHANNEL_1
#define RIGHT_TIM    htim3
#define RIGHT_CH     TIM_CHANNEL_2
#define MAX_FREQ_HZ  25000u

/* Manual ramp tuning (edit here) */
static uint32_t g_step_period_ms = 10u;  /* update period */
static float    g_step_pct       = 1.0f; /* percent per step; <=0 => no ramp (jump) */

static float s_curL = 0.0f, s_curR = 0.0f;
static float s_tgtL = 0.0f, s_tgtR = 0.0f;
static uint32_t s_lastTick = 0u;

static bool s_runL = false;
static bool s_runR = false;

typedef enum { MOTOR_FWD = 0, MOTOR_REV = 1 } motor_dir_t;

static motor_dir_t s_dirL = MOTOR_FWD;
static motor_dir_t s_dirR = MOTOR_FWD;

// Returns the effective APB1 timer clock (TIM2/TIM3)
static uint32_t tim_apb1_clk_hz(void)
{
    RCC_ClkInitTypeDef clk = {0};
    uint32_t lat;
    HAL_RCC_GetClockConfig(&clk, &lat);

    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    return (clk.APB1CLKDivider == RCC_HCLK_DIV1) ? pclk1 : (2u * pclk1);
}

// Computes PSC and ARR values so that the timer generates a PWM signal with the requested frequency (pwm_hz), respecting 16-bit timer limits.
static void compute_psc_arr(uint32_t pwm_hz, uint32_t *psc, uint32_t *arr)
{
    uint32_t clk = tim_apb1_clk_hz();
    if (pwm_hz < 1u) pwm_hz = 1u;

    for (uint32_t p = 0; p <= 0xFFFFu; p++)
    {
        uint32_t denom = (p + 1u) * pwm_hz;
        if (denom == 0u) continue;

        uint32_t a = (clk / denom);
        if (a == 0u) continue;
        a -= 1u;

        if (a <= 0xFFFFu) { *psc = p; *arr = a; return; }
    }

    *psc = 0xFFFFu;
    *arr = 0xFFFFu;
}

// Maps 0..100% to 0..MAX_FREQ_HZ (0% => stop PWM).
static uint32_t pct_to_freq_hz(float pct)
{
    if (pct <= 0.0f)  return 0u;
    if (pct >= 100.0f) return MAX_FREQ_HZ;

    float f = (pct * (float)MAX_FREQ_HZ) / 100.0f;
    if (f < 1.0f) f = 1.0f;
    return (uint32_t)(f + 0.5f);
}

// Sets PWM frequency on a given timer/channel with fixed 50% duty, and starts/stops output as needed.
static void pwm_set_freq_50pct(TIM_HandleTypeDef *htim, uint32_t channel, bool *running, uint32_t freq_hz)
{
    if (freq_hz == 0u) {
        if (*running) {
            HAL_TIM_PWM_Stop(htim, channel);
            *running = false;
        }
        return;
    }

    uint32_t psc, arr;
    compute_psc_arr(freq_hz, &psc, &arr);

    __HAL_TIM_SET_PRESCALER(htim, psc);
    __HAL_TIM_SET_AUTORELOAD(htim, arr);

    __HAL_TIM_SET_COMPARE(htim, channel, (arr + 1u) / 2u); /* 50% */
    __HAL_TIM_SET_COUNTER(htim, 0u);
    HAL_TIM_GenerateEvent(htim, TIM_EVENTSOURCE_UPDATE);

    if (!*running) {
        HAL_TIM_PWM_Start(htim, channel);
        *running = true;
    }
}

// Directions control
static void motor_set_dir_left(motor_dir_t dir)
{
    HAL_GPIO_WritePin(L_DIR_GPIO_Port, L_DIR_Pin,
                      (dir == MOTOR_REV) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void motor_set_dir_right(motor_dir_t dir)
{
    HAL_GPIO_WritePin(R_DIR_GPIO_Port, R_DIR_Pin,
                      (dir == MOTOR_REV) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Driver enable control
void Motors_SetEnable(bool enable)
{
    HAL_GPIO_WritePin(L_EN_GPIO_Port, L_EN_Pin,
                      enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin,
                      enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Initializes the motor PWM control state: stops TIM2/TIM3 PWM outputs, resets ramp variables, and captures the initial tick time.
void Motors_Init(void)
{
    HAL_TIM_PWM_Stop(&LEFT_TIM,  LEFT_CH);
    HAL_TIM_PWM_Stop(&RIGHT_TIM, RIGHT_CH);
    s_runL = false;
    s_runR = false;

    s_curL = s_curR = 0.0f;
    s_tgtL = s_tgtR = 0.0f;
    s_lastTick = HAL_GetTick();

    s_dirL = MOTOR_FWD;
    s_dirR = MOTOR_FWD;
    motor_set_dir_left(s_dirL);
    motor_set_dir_right(s_dirR);

    Motors_SetEnable(true);
}

// Updates left/right "speed" in percent (0% => stop, 100% => 25 kHz, -100% => reverse 25kHz) using ramp.
void Motors_Speed_inPercent(float left_pct, float right_pct)
{
    motor_dir_t reqDirL = (left_pct  < 0.0f) ? MOTOR_REV : MOTOR_FWD;
    motor_dir_t reqDirR = (right_pct < 0.0f) ? MOTOR_REV : MOTOR_FWD;

    float absL = (left_pct  < 0.0f) ? -left_pct  : left_pct;
    float absR = (right_pct < 0.0f) ? -right_pct : right_pct;

    if (absL > 100.0f) absL = 100.0f;
    if (absR > 100.0f) absR = 100.0f;

    bool wantFlipL = (reqDirL != s_dirL);
    bool wantFlipR = (reqDirR != s_dirR);

    if (wantFlipL && s_curL > 0.0f) absL = 0.0f;
    if (wantFlipR && s_curR > 0.0f) absR = 0.0f;

    s_tgtL = absL;
    s_tgtR = absR;

    uint32_t now = HAL_GetTick();
    if ((now - s_lastTick) < g_step_period_ms) return;
    s_lastTick = now;

    float step = g_step_pct;
    if (step <= 0.0f) {
        s_curL = s_tgtL;
        s_curR = s_tgtR;
    } else {
        if (s_curL < s_tgtL) { s_curL += step; if (s_curL > s_tgtL) s_curL = s_tgtL; }
        else if (s_curL > s_tgtL) { s_curL -= step; if (s_curL < s_tgtL) s_curL = s_tgtL; }

        if (s_curR < s_tgtR) { s_curR += step; if (s_curR > s_tgtR) s_curR = s_tgtR; }
        else if (s_curR > s_tgtR) { s_curR -= step; if (s_curR < s_tgtR) s_curR = s_tgtR; }
    }

    if (wantFlipL && s_curL == 0.0f) {
        s_dirL = reqDirL;
        motor_set_dir_left(s_dirL);
    }
    if (wantFlipR && s_curR == 0.0f) {
        s_dirR = reqDirR;
        motor_set_dir_right(s_dirR);
    }

    pwm_set_freq_50pct(&LEFT_TIM,  LEFT_CH,  &s_runL, pct_to_freq_hz(s_curL));
    pwm_set_freq_50pct(&RIGHT_TIM, RIGHT_CH, &s_runR, pct_to_freq_hz(s_curR));
}

// Update motors speed via controls
void Motors_Control(uint8_t keys_state) {
	float left  = 0.0f;
	float right = 0.0f;

	if (g_keys_state & KEY_W) { 		// forward
		left  = -1.0f;
		right = 1.0f;
	} else if (g_keys_state & KEY_S) {  // reverse
		left  = 1.0f;
		right = -1.0f;
	} else if (g_keys_state & KEY_A) {  // turn left
		left  = 0.5f;
		right = 0.5f;
	} else if (g_keys_state & KEY_D) {  // turn right
		left  = -0.5f;
		right = -0.5f;
	}

	Motors_Speed_inPercent(left, right);
}

