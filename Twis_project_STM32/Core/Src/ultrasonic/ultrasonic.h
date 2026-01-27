/*
 * ultrasonic.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_ULTRASONIC_ULTRASONIC_H_
#define SRC_ULTRASONIC_ULTRASONIC_H_

#include "stm32f303x8.h"
#include <stdint.h>

/* vÃ½sledok merania v metroch */
extern float distance_m;

/* PB3 = TRIG, PB4 = ECHO */
void Ultrasonic_Init(void);
float Ultrasonic_UpdateDistance(void);

#endif /* SRC_ULTRASONIC_ULTRASONIC_H_ */
