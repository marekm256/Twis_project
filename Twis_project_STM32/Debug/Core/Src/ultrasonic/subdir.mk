################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ultrasonic/ultrasonic.c 

OBJS += \
./Core/Src/ultrasonic/ultrasonic.o 

C_DEPS += \
./Core/Src/ultrasonic/ultrasonic.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ultrasonic/%.o Core/Src/ultrasonic/%.su Core/Src/ultrasonic/%.cyclo: ../Core/Src/ultrasonic/%.c Core/Src/ultrasonic/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ultrasonic

clean-Core-2f-Src-2f-ultrasonic:
	-$(RM) ./Core/Src/ultrasonic/ultrasonic.cyclo ./Core/Src/ultrasonic/ultrasonic.d ./Core/Src/ultrasonic/ultrasonic.o ./Core/Src/ultrasonic/ultrasonic.su

.PHONY: clean-Core-2f-Src-2f-ultrasonic

