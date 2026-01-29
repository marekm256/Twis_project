/*
 * ultrasonic_hcsr04.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_
#define SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_

#include "stm32f3xx_hal.h"   // uprav, ak máš inú STM32 sériu

void Ultrasonic_Init(TIM_HandleTypeDef *htim);
float Ultrasonic_ReadDistanceCM(void);   // vráti metre, -1 pri chybe

#endif /* SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_ */
