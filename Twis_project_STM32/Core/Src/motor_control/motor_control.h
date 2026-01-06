/*
 * motor_control.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_
#define SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_

#include <stdint.h>

<<<<<<< HEAD
void Motor_Init(void);
=======
/* API pre pravý motor */
void Motor_Init(uint32_t pwm_freq_hz);
>>>>>>> f0626c1e1b0ce432afd47d0ec7be18ae41c45531
void Motor_Task(void);

#endif /* SRC_MOTOR_CONTROL_MOTOR_CONTROL_H_ */

