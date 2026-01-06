#ifndef SRC_IMU_MPU6050_IMU_MPU6050_H_
#define SRC_IMU_MPU6050_IMU_MPU6050_H_

typedef struct {
    float angle_x;   // roll
    float angle_y;   // pitch

    float gyro_x;
    float gyro_y;
    float gyro_z;    // ✅ pridane

    float acc_x;
    float acc_y;
    float acc_z;     // ✅ pridane
} IMU_Data_t;

void IMU_Init(void);
void IMU_Read(IMU_Data_t *data);

#endif /* SRC_IMU_MPU6050_IMU_MPU6050_H_ */
