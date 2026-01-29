################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/comm_rpizero2w/comm_rpizero2w.c 

OBJS += \
./Core/Src/comm_rpizero2w/comm_rpizero2w.o 

C_DEPS += \
./Core/Src/comm_rpizero2w/comm_rpizero2w.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/comm_rpizero2w/%.o Core/Src/comm_rpizero2w/%.su Core/Src/comm_rpizero2w/%.cyclo: ../Core/Src/comm_rpizero2w/%.c Core/Src/comm_rpizero2w/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Core/Src/comm_rpizero2w -I../Core/Src/motors_24h055m020 -I../Core/Src/ultrasonic_hcsr04 -I../Core/Src/imu_mpu6500 -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-comm_rpizero2w

clean-Core-2f-Src-2f-comm_rpizero2w:
	-$(RM) ./Core/Src/comm_rpizero2w/comm_rpizero2w.cyclo ./Core/Src/comm_rpizero2w/comm_rpizero2w.d ./Core/Src/comm_rpizero2w/comm_rpizero2w.o ./Core/Src/comm_rpizero2w/comm_rpizero2w.su

.PHONY: clean-Core-2f-Src-2f-comm_rpizero2w

