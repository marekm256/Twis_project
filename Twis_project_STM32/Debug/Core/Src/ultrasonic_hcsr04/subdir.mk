################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.c 

OBJS += \
./Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.o 

C_DEPS += \
./Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ultrasonic_hcsr04/%.o Core/Src/ultrasonic_hcsr04/%.su Core/Src/ultrasonic_hcsr04/%.cyclo: ../Core/Src/ultrasonic_hcsr04/%.c Core/Src/ultrasonic_hcsr04/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Core/Src/comm_rpizero2w -I../Core/Src/motors_24h055m020 -I../Core/Src/ultrasonic_hcsr04 -I../Core/Src/imu_mpu6500 -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ultrasonic_hcsr04

clean-Core-2f-Src-2f-ultrasonic_hcsr04:
	-$(RM) ./Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.cyclo ./Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.d ./Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.o ./Core/Src/ultrasonic_hcsr04/ultrasonic_hcsr04.su

.PHONY: clean-Core-2f-Src-2f-ultrasonic_hcsr04

