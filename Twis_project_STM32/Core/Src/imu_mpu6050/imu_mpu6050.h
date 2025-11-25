/*
 * imu_mpu6050.h
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#ifndef SRC_IMU_MPU6050_IMU_MPU6050_H_
#define SRC_IMU_MPU6050_IMU_MPU6050_H_

typedef struct {
    float angle_x;
    float angle_y;
    float gyro_x;
    float gyro_y;
    float acc_x;
    float acc_y;
} IMU_Data_t;

void IMU_Init(void);
void IMU_Read(IMU_Data_t *data);

#endif /* SRC_IMU_MPU6050_IMU_MPU6050_H_ */
