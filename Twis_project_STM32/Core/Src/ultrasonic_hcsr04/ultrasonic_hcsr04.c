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

/* ECHO = PA8 -> TIM1_CH1 (Input Capture) */
#define ECHO_IC_CHANNEL TIM_CHANNEL_1

/* Speed of sound ~343 m/s => 0.0343 cm/us
   Distance(cm) = pulse_us * 0.0343 / 2 = pulse_us / 58.2
*/
#define US_TO_CM_DIV 58.2f

static TIM_HandleTypeDef *u_htim = NULL;

static volatile uint32_t ic_rise = 0;
static volatile uint32_t ic_fall = 0;
static volatile uint8_t  got_rise = 0;
static volatile uint8_t  got_fall = 0;

/* krátke oneskorenie v µs (timer musí bežať 1 MHz) */
static void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(u_htim, 0);
    while (__HAL_TIM_GET_COUNTER(u_htim) < us) { }
}

void Ultrasonic_Init(TIM_HandleTypeDef *htim_ic)
{
    u_htim = htim_ic;

    got_rise = 0;
    got_fall = 0;

    /* spusti timer + input capture interrupt */
    HAL_TIM_IC_Start_IT(u_htim, ECHO_IC_CHANNEL);

    /* TRIG low */
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}

void Ultrasonic_Trigger(void)
{
    /* vymaž stav merania */
    got_rise = 0;
    got_fall = 0;

    /* aby bol ECHO stabilný */
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    delay_us(2);

    /* 10 us pulz */
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}

/**
 * Dôležité:
 * Tento callback musíš mať v projekte len raz.
 * Ak už máš HAL_TIM_IC_CaptureCallback inde, tak túto logiku prenes tam.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (u_htim == NULL) return;
    if (htim->Instance != u_htim->Instance) return;

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        if (!got_rise)
        {
            /* Rising edge */
            ic_rise = HAL_TIM_ReadCapturedValue(htim, ECHO_IC_CHANNEL);
            got_rise = 1;

            /* prepni polaritu na Falling */
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, ECHO_IC_CHANNEL, TIM_INPUTCHANNELPOLARITY_FALLING);
        }
        else if (!got_fall)
        {
            /* Falling edge */
            ic_fall = HAL_TIM_ReadCapturedValue(htim, ECHO_IC_CHANNEL);
            got_fall = 1;

            /* priprav ďalšie meranie zas na Rising */
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, ECHO_IC_CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);
        }
    }
}

float Ultrasonic_ReadDistanceCm(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();

    Ultrasonic_Trigger();

    /* čakaj na obe hrany */
    while (!(got_rise && got_fall))
    {
        if ((HAL_GetTick() - start) > timeout_ms)
        {
            /* reset polaritu pre istotu */
            __HAL_TIM_SET_CAPTUREPOLARITY(u_htim, ECHO_IC_CHANNEL, TIM_INPUTCHANNELPOLARITY_RISING);
            return -1.0f;
        }
    }

    /* vypočítaj šírku pulzu v tickoch */
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(u_htim);
    uint32_t pulse;

    if (ic_fall >= ic_rise)
        pulse = ic_fall - ic_rise;
    else
        pulse = (arr - ic_rise + 1u) + ic_fall;

    /* ak timer beží na 1 MHz => 1 tick = 1 us */
    float distance_cm = (float)pulse / US_TO_CM_DIV;
    return distance_cm;
}
