################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/imu_mpu6050/imu_mpu6050.c 

OBJS += \
./Core/Src/imu_mpu6050/imu_mpu6050.o 

C_DEPS += \
./Core/Src/imu_mpu6050/imu_mpu6050.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/imu_mpu6050/%.o Core/Src/imu_mpu6050/%.su Core/Src/imu_mpu6050/%.cyclo: ../Core/Src/imu_mpu6050/%.c Core/Src/imu_mpu6050/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-imu_mpu6050

clean-Core-2f-Src-2f-imu_mpu6050:
	-$(RM) ./Core/Src/imu_mpu6050/imu_mpu6050.cyclo ./Core/Src/imu_mpu6050/imu_mpu6050.d ./Core/Src/imu_mpu6050/imu_mpu6050.o ./Core/Src/imu_mpu6050/imu_mpu6050.su

.PHONY: clean-Core-2f-Src-2f-imu_mpu6050

