################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/comm/comm.c 

OBJS += \
./Core/Src/comm/comm.o 

C_DEPS += \
./Core/Src/comm/comm.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/comm/%.o Core/Src/comm/%.su Core/Src/comm/%.cyclo: ../Core/Src/comm/%.c Core/Src/comm/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Core/Src/comm -I../Core/Src/motors_24h055m020 -I../Core/Src/ultrasonic -I../Core/Src/imu_mpu6500 -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-comm

clean-Core-2f-Src-2f-comm:
	-$(RM) ./Core/Src/comm/comm.cyclo ./Core/Src/comm/comm.d ./Core/Src/comm/comm.o ./Core/Src/comm/comm.su

.PHONY: clean-Core-2f-Src-2f-comm

