/*
 * imu_mpu6500.c
 *
 *  Created on: Nov 25, 2025
 *      Author: malin
 */

#include "imu_mpu6500.h"
#include "i2c.h"

#include "stm32f3xx_hal.h"
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define MPU6500_ADDR7          (0x68u)
#define MPU6500_ADDR8          ((uint16_t)(MPU6500_ADDR7 << 1))


#define REG_WHO_AM_I           (0x75u)
#define REG_PWR_MGMT_1         (0x6Bu)

#define REG_SMPLRT_DIV         (0x19u)
#define REG_CONFIG             (0x1Au)
#define REG_GYRO_CONFIG        (0x1Bu)
#define REG_ACCEL_CONFIG       (0x1Cu)
#define REG_ACCEL_CONFIG2      (0x1Du)

#define REG_ACCEL_XOUT_H       (0x3Bu)


#define I2C_TIMEOUT_MS         (50u)


#define IMU_AVG_N              (5)

static bool     s_inited = false;
static uint8_t  s_who = 0xFF;


static float s_gx_bias = 0.0f;
static float s_gy_bias = 0.0f;
static float s_gz_bias = 0.0f;


static float s_gyro_lsb_per_dps = 16.4f;
static float s_accel_lsb_per_g  = 8192.0f;


typedef struct {
  float buf[IMU_AVG_N];
  uint8_t idx;
  uint8_t cnt;
  float max_diff;
} AvgFilter;

static AvgFilter f_ay_deg;

static void filter_reset(AvgFilter *f)
{
  for (int i = 0; i < IMU_AVG_N; i++) f->buf[i] = 0.0f;
  f->idx = 0;
  f->cnt = 0;

}

static void filter_prefill(AvgFilter *f, float val)
{
  for (int i = 0; i < IMU_AVG_N; i++) f->buf[i] = val;
  f->idx = 0;
  f->cnt = IMU_AVG_N;
}

static float filter_update(AvgFilter *f, float new_val)
{
  float avg = new_val;

  f->buf[f->idx] = new_val;
  f->idx = (uint8_t)((f->idx + 1u) % IMU_AVG_N);
  if (f->cnt < IMU_AVG_N) f->cnt++;

  float sum = 0.0f;
  for (int i = 0; i < f->cnt; i++) sum += f->buf[i];
  return sum / (float)f->cnt;
}


static HAL_StatusTypeDef imu_read_u8(uint8_t reg, uint8_t *val)
{
  return HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR8, reg, I2C_MEMADD_SIZE_8BIT,
                          val, 1, I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef imu_write_u8(uint8_t reg, uint8_t val)
{
  return HAL_I2C_Mem_Write(&hi2c1, MPU6500_ADDR8, reg, I2C_MEMADD_SIZE_8BIT,
                           &val, 1, I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef imu_read_bytes(uint8_t reg, uint8_t *buf, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR8, reg, I2C_MEMADD_SIZE_8BIT,
                          buf, len, I2C_TIMEOUT_MS);
}

static bool whoami_ok(uint8_t w)
{

  return (w == 0x70u) || (w == 0x71u) || (w == 0x68u);
}


static HAL_StatusTypeDef imu_read_raw_burst(int16_t* ax, int16_t* ay, int16_t* az,
                                           int16_t* gx, int16_t* gy, int16_t* gz,
                                           int16_t* temp)
{
  uint8_t b[14];
  HAL_StatusTypeDef st = imu_read_bytes(REG_ACCEL_XOUT_H, b, (uint16_t)sizeof(b));
  if (st != HAL_OK) return st;

  *ax = (int16_t)((b[0] << 8) | b[1]);
  *ay = (int16_t)((b[2] << 8) | b[3]);
  *az = (int16_t)((b[4] << 8) | b[5]);

  *temp = (int16_t)((b[6] << 8) | b[7]);

  *gx = (int16_t)((b[8]  << 8) | b[9]);
  *gy = (int16_t)((b[10] << 8) | b[11]);
  *gz = (int16_t)((b[12] << 8) | b[13]);

  return HAL_OK;
}


uint8_t IMU_IsReady(void)   { return s_inited ? 1u : 0u; }
uint8_t IMU_GetWhoAmI(void) { return s_who; }

HAL_StatusTypeDef IMU_Init(void)
{
  s_inited = false;
  s_who = 0xFF;


  (void)imu_write_u8(REG_PWR_MGMT_1, 0x00u);
  HAL_Delay(10);


  uint8_t w = 0;
  if (imu_read_u8(REG_WHO_AM_I, &w) != HAL_OK) return HAL_ERROR;
  if (!whoami_ok(w)) return HAL_ERROR;
  s_who = w;


  (void)imu_write_u8(REG_CONFIG, 0x03u);
  (void)imu_write_u8(REG_SMPLRT_DIV, 4u);

  (void)imu_write_u8(REG_GYRO_CONFIG, (uint8_t)(3u << 3));
  s_gyro_lsb_per_dps = 16.4f;

  (void)imu_write_u8(REG_ACCEL_CONFIG, (uint8_t)(1u << 3));
  s_accel_lsb_per_g = 8192.0f;

  (void)imu_write_u8(REG_ACCEL_CONFIG2, 0x03u);


  s_gx_bias = 0.0f; s_gy_bias = 0.0f; s_gz_bias = 0.0f;

  const int N = 200;
  const uint32_t sample_delay_ms = 5;
  int good = 0;

  for (int i = 0; i < N; i++) {
    int16_t ax, ay, az, gx, gy, gz, temp;
    if (imu_read_raw_burst(&ax, &ay, &az, &gx, &gy, &gz, &temp) == HAL_OK) {
      s_gx_bias += (float)gx / s_gyro_lsb_per_dps;
      s_gy_bias += (float)gy / s_gyro_lsb_per_dps;
      s_gz_bias += (float)gz / s_gyro_lsb_per_dps;
      good++;
    }
    HAL_Delay(sample_delay_ms);
  }

  if (good > 0) {
    s_gx_bias /= (float)good;
    s_gy_bias /= (float)good;
    s_gz_bias /= (float)good;
  }


  filter_reset(&f_ay_deg);

  {
    int16_t ax, ay, az, gx, gy, gz, temp;
    if (imu_read_raw_burst(&ax, &ay, &az, &gx, &gy, &gz, &temp) == HAL_OK) {
      float ay_g = (float)ay / s_accel_lsb_per_g;
      float ay_deg = ay_g * 90.0f;
      filter_prefill(&f_ay_deg, ay_deg);
    } else {
      filter_prefill(&f_ay_deg, 0.0f);
    }
  }

  s_inited = true;
  return HAL_OK;
}

HAL_StatusTypeDef IMU_ReadRaw(IMU_Raw *out)
{
  if (!out) return HAL_ERROR;
  if (!s_inited) return HAL_ERROR;

  int16_t ax, ay, az, gx, gy, gz, temp;
  HAL_StatusTypeDef st = imu_read_raw_burst(&ax, &ay, &az, &gx, &gy, &gz, &temp);
  if (st != HAL_OK) return st;

  out->ax = ax; out->ay = ay; out->az = az;
  out->gx = gx; out->gy = gy; out->gz = gz;
  out->temp = temp;

  return HAL_OK;
}

HAL_StatusTypeDef IMU_ReadData(IMU_Data *out)
{
  if (!out) return HAL_ERROR;
  if (!s_inited) return HAL_ERROR;

  IMU_Raw r;
  HAL_StatusTypeDef st = IMU_ReadRaw(&r);
  if (st != HAL_OK) return st;


  out->ax_g = (float)r.ax / s_accel_lsb_per_g;
  out->ay_g = (float)r.ay / s_accel_lsb_per_g;
  out->az_g = (float)r.az / s_accel_lsb_per_g;
  out->gx_dps = ((float)r.gx / s_gyro_lsb_per_dps) - s_gx_bias;
  out->gy_dps = ((float)r.gy / s_gyro_lsb_per_dps) - s_gy_bias;
  out->gz_dps = ((float)r.gz / s_gyro_lsb_per_dps) - s_gz_bias;
  out->temp_c = ((float)r.temp) / 333.87f + 21.0f;


  float ay_deg = out->ay_g * 90.0f;


  out->roll_degmean = filter_update(&f_ay_deg, ay_deg);

  out->roll_deg = ay_deg;

  return HAL_OK;
}
