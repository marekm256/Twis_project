/*
 * ultrasonic.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

// ultrasonic.c
#include "ultrasonic.h"

/*
  HC-SR04 zapojenie:
    TRIG -> PB3
    ECHO -> PB4   (cez delič 5V -> 3.3V)
*/

float distance_m = 0.0f;

/* piny */
#define TRIG_PIN        3U   /* PB3 */
#define ECHO_PIN        4U   /* PB4 */

#define TRIG_MASK       (1U << TRIG_PIN)
#define ECHO_MASK       (1U << ECHO_PIN)

#define ECHO_TIMEOUT_US 30000U   /* 30 ms */

/* ---------- TIM1: 1 µs tick ---------- */
static void tim1_init_1us(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    TIM1->PSC = 71;          /* 72 MHz / (71+1) = 1 MHz */
    TIM1->ARR = 0xFFFF;
    TIM1->CNT = 0;
    TIM1->CR1 |= TIM_CR1_CEN;
}

static void delay_us(uint16_t us)
{
    TIM1->CNT = 0;
    while (TIM1->CNT < us) { }
}

/* wait for ECHO HIGH/LOW with timeout
   return 1 = timeout, 0 = OK
*/
static uint8_t wait_echo_state(uint8_t want_high, uint32_t timeout_us)
{
    TIM1->CNT = 0;

    if (want_high) {
        while ((GPIOB->IDR & ECHO_MASK) == 0U) {
            if (TIM1->CNT > timeout_us) return 1;
        }
    } else {
        while ((GPIOB->IDR & ECHO_MASK) != 0U) {
            if (TIM1->CNT > timeout_us) return 1;
        }
    }
    return 0;
}

/* ---------- INIT ---------- */
void Ultrasonic_Init(void)
{
    /* GPIOB clock */
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    /* PB3 = TRIG (output) */
    GPIOB->MODER &= ~(3U << (TRIG_PIN * 2U));
    GPIOB->MODER |=  (1U << (TRIG_PIN * 2U));

    /* PB4 = ECHO (input) */
    GPIOB->MODER &= ~(3U << (ECHO_PIN * 2U));

    /* TRIG low */
    GPIOB->ODR &= ~TRIG_MASK;

    tim1_init_1us();
}

/* ---------- MERANIE ---------- */
float Ultrasonic_UpdateDistance(void)
{
    uint32_t time_us;

    /* TRIG pulse 10 us */
    GPIOB->ODR &= ~TRIG_MASK;
    delay_us(2);
    GPIOB->ODR |=  TRIG_MASK;
    delay_us(10);
    GPIOB->ODR &= ~TRIG_MASK;

    /* wait ECHO HIGH */
    if (wait_echo_state(1, ECHO_TIMEOUT_US)) {
        distance_m = -1.0f;
        return 0.0f;
    }

    /* measure HIGH width */
    TIM1->CNT = 0;
    if (wait_echo_state(0, ECHO_TIMEOUT_US)) {
        distance_m = -1.0f;
        return 0.0f;
    }

    time_us = TIM1->CNT;

    /* prepočet na metre: m ≈ us / 5800 */
    return (float)time_us / 5800.0f;
}
