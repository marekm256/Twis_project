/*
 * ultrasonic_hcsr04.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_
#define SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_

#include "stm32f3xx_hal.h"   // uprav podľa svojej série (f0/f1/f3/f4...)

void Ultrasonic_Init(TIM_HandleTypeDef *htim_ic);
void Ultrasonic_Trigger(void);

/**
 * Blocking čítanie vzdialenosti.
 * @param timeout_ms max čas čakania na meranie
 * @return vzdialenosť v cm, alebo -1 pri timeout / chybe
 */
float Ultrasonic_ReadDistanceCm(uint32_t timeout_ms);


#endif /* SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_ */
