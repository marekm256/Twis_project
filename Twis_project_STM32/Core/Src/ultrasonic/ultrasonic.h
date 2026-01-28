/*
 * ultrasonic.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_ULTRASONIC_ULTRASONIC_H_
#define SRC_ULTRASONIC_ULTRASONIC_H_

#include "main.h"   // má stm32f3xx_hal.h

// init (spusti TIM1)
void Ultrasonic_Init(TIM_HandleTypeDef *htim);

// vzdialenosť v metroch; pri chybe/timeout vráti -1.0f
float Ultrasonic_ReadDistanceM(void);


#endif /* SRC_ULTRASONIC_ULTRASONIC_H_ */
