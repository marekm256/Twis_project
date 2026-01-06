/*
 * imu_mpu6050.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#include "imu_mpu6050.h"
#include "i2c.h"
#include "usart.h"

#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* 7-bit addr 0x68, HAL expects 8-bit (shifted) */
#define MPU6050_ADDR               (0x68 << 1)

#define MPU6050_WHO_AM_I           0x75
#define MPU6050_PWR_MGMT_1         0x6B
#define MPU6050_PWR_MGMT_2         0x6C

#define MPU6050_GYRO_CONFIG        0x1B
#define MPU6050_ACCEL_CONFIG       0x1C

#define MPU6050_ACCEL_XOUT_H       0x3B

#define MPU6050_USER_CTRL          0x6A
#define MPU6050_SIGNAL_PATH_RESET  0x68

/* --- local UART helper --- */
static void imu_uart_print(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}

void IMU_Init(void)
{
    uint8_t data;
    uint8_t who_am_i = 0;
    char msg[64];

    /* 1) Clock source = PLL with X gyro */
    data = 0x01;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR,
                      MPU6050_PWR_MGMT_1,
                      I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);
    HAL_Delay(100);

    /* 2) Disable I2C master, FIFO, DMP */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR,
                      MPU6050_USER_CTRL,
                      I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);

    /* 3) Reset sensor signal paths (gyro+accel+temp) */
    data = 0x07;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR,
                      MPU6050_SIGNAL_PATH_RESET,
                      I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);
    HAL_Delay(100);

    /* 4) Enable all accel + gyro axes */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR,
                      MPU6050_PWR_MGMT_2,
                      I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);

    /* 5) Gyro range ±250 dps (FS_SEL=0) */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR,
                      MPU6050_GYRO_CONFIG,
                      I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);

    /* 6) Accel range ±2g (AFS_SEL=0) */
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR,
                      MPU6050_ACCEL_CONFIG,
                      I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);

    /* 7) WHO_AM_I check */
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR,
                     MPU6050_WHO_AM_I,
                     I2C_MEMADD_SIZE_8BIT,
                     &who_am_i, 1, HAL_MAX_DELAY);

    snprintf(msg, sizeof(msg), "MPU6050 WHO_AM_I = 0x%02X\r\n", who_am_i);
    imu_uart_print(msg);
}

void IMU_Read(IMU_Data_t *data)
{
    uint8_t raw[14];
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(&hi2c1,
                              MPU6050_ADDR,
                              MPU6050_ACCEL_XOUT_H,
                              I2C_MEMADD_SIZE_8BIT,
                              raw,
                              14,
                              HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        imu_uart_print("IMU: I2C READ ERROR\r\n");
        return;
    }

    /* RAW values */
    int16_t ax = (int16_t)((raw[0] << 8) | raw[1]);
    int16_t ay = (int16_t)((raw[2] << 8) | raw[3]);
    int16_t az = (int16_t)((raw[4] << 8) | raw[5]);

    int16_t gx = (int16_t)((raw[8]  << 8) | raw[9]);
    int16_t gy = (int16_t)((raw[10] << 8) | raw[11]);
    int16_t gz = (int16_t)((raw[12] << 8) | raw[13]);

    /* Store raw as float (for math) */
    data->acc_x = (float)ax;
    data->acc_y = (float)ay;
    data->acc_z = (float)az;

    data->gyro_x = (float)gx;
    data->gyro_y = (float)gy;
    data->gyro_z = (float)gz;

    /* Compute pitch/roll from accelerometer only */
    float axf = data->acc_x;
    float ayf = data->acc_y;
    float azf = data->acc_z;

    float pitch_f = atan2f(axf, sqrtf(ayf*ayf + azf*azf)) * (180.0f / 3.14159265f);
    float roll_f  = atan2f(ayf, azf) * (180.0f / 3.14159265f);

    data->angle_x = roll_f;
    data->angle_y = pitch_f;

    /* Tilt state */
    const float TH = 10.0f;
    const char *state;

    if (pitch_f > TH)       state = "NAKLON DOPREDU";
    else if (pitch_f < -TH) state = "NAKLON DOZADU";
    else                    state = "OK";


    char msg[140];
    int pitch_i = (int)pitch_f;
    int roll_i  = (int)roll_f;

    snprintf(msg, sizeof(msg),
             "P=%ddeg R=%ddeg | %s | ACC[%d %d %d] GYRO[%d %d %d]\r\n",
             pitch_i, roll_i, state,
             ax, ay, az, gx, gy, gz);

    imu_uart_print(msg);
}
