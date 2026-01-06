/*
 * motor_control.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_
#define SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_

#include <stdint.h>

/* API pre pravý motor */
void Motor_Init(uint32_t pwm_freq_hz);
void Motor_Task(void);

#endif /* SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_ */
