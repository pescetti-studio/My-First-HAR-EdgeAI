################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/activation.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/lsm6dsox_reg.c \
../Core/Src/main.c \
../Core/Src/model.c \
../Core/Src/model_weights.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscall.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/activation.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/lsm6dsox_reg.o \
./Core/Src/main.o \
./Core/Src/model.o \
./Core/Src/model_weights.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscall.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/activation.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/lsm6dsox_reg.d \
./Core/Src/main.d \
./Core/Src/model.d \
./Core/Src/model_weights.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscall.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM4 -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/croci/STM32CubeIDE/workspace_1.18.1/progetto_SE_croci/Drivers/BSP/Components/lsm6dsox" -I"C:/Users/croci/STM32CubeIDE/workspace_1.18.1/progetto_SE_croci/Drivers/CMSIS/Include" -I"C:/Users/croci/STM32CubeIDE/workspace_1.18.1/progetto_SE_croci/Drivers/STM32F4xx_HAL_Driver/Src" -I"C:/Users/croci/STM32CubeIDE/workspace_1.18.1/progetto_SE_croci/Drivers/CMSIS/Device" -I"C:/Users/croci/STM32CubeIDE/workspace_1.18.1/progetto_SE_croci/Drivers/CMSIS/DSP/Include" -I"C:/Users/croci/STM32CubeIDE/workspace_1.18.1/progetto_SE_croci/Drivers/CMSIS/Lib/ARM" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/activation.cyclo ./Core/Src/activation.d ./Core/Src/activation.o ./Core/Src/activation.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/lsm6dsox_reg.cyclo ./Core/Src/lsm6dsox_reg.d ./Core/Src/lsm6dsox_reg.o ./Core/Src/lsm6dsox_reg.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/model.cyclo ./Core/Src/model.d ./Core/Src/model.o ./Core/Src/model.su ./Core/Src/model_weights.cyclo ./Core/Src/model_weights.d ./Core/Src/model_weights.o ./Core/Src/model_weights.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscall.cyclo ./Core/Src/syscall.d ./Core/Src/syscall.o ./Core/Src/syscall.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

