/*
 * motor_control.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_
#define SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_

#include <stdint.h>

void Motors_Init(void);
void Motors_Speed_inPercent(float left_pct, float right_pct); //  Signed percent: -100..+100

#endif /* SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_ */
