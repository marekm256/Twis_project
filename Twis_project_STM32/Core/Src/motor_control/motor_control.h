/*
 * motor_control.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_
#define SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_

void Motor_Init(void);
void Motor_Task(void);
void Motor_SetDuty(int16_t left, int16_t right);

#endif /* SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_ */
