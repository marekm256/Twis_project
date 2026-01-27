################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/gyro/gyro.c 

OBJS += \
./Core/Src/gyro/gyro.o 

C_DEPS += \
./Core/Src/gyro/gyro.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/gyro/%.o Core/Src/gyro/%.su Core/Src/gyro/%.cyclo: ../Core/Src/gyro/%.c Core/Src/gyro/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Core/Src/comm -I../Core/Src/motor_control -I../Core/Src/ultrasonic -I../Core/Src/gyro -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-gyro

clean-Core-2f-Src-2f-gyro:
	-$(RM) ./Core/Src/gyro/gyro.cyclo ./Core/Src/gyro/gyro.d ./Core/Src/gyro/gyro.o ./Core/Src/gyro/gyro.su

.PHONY: clean-Core-2f-Src-2f-gyro

