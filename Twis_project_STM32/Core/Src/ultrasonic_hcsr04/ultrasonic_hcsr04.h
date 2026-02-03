/*
 * ultrasonic_hcsr04.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_
#define SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_

#include "stm32f3xx_hal.h"   // ak máš inú STM32 sériu, zmeň include

void Ultrasonic_Init(TIM_HandleTypeDef *htim);

/* Zachovaná pôvodná funkcia (len opravíme jednotky):
   vráti vzdialenosť v cm, -1.0f pri chybe
*/
float Ultrasonic_ReadDistanceCM(void);

/* NOVÁ funkcia navyše: priemer z N meraní v cm
   vráti -1.0f ak nebolo žiadne platné meranie
*/
float Ultrasonic_ReadDistanceAvg(uint8_t samples);

#endif /* SRC_ULTRASONIC_HCSR04_ULTRASONIC_H_ */
