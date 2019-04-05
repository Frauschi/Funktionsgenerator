################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../System/SystemManager.cpp 

OBJS += \
./System/SystemManager.o 

CPP_DEPS += \
./System/SystemManager.d 


# Each subdirectory must supply rules for building sources it contributes
System/%.o: ../System/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: MCU G++ Compiler'
	@echo $(PWD)
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -std=c++1y -DSTM32 -DSTM32L4 -DSTM32L476RGTx -DNUCLEO_L476RG -I"/home/tobias/eclipse-workspace/waveformgenerator2/CMSIS" -I"/home/tobias/eclipse-workspace/waveformgenerator2/Util" -I"/home/tobias/eclipse-workspace/waveformgenerator2/Device" -I"/home/tobias/eclipse-workspace/waveformgenerator2/Driver" -I"/home/tobias/eclipse-workspace/waveformgenerator2/Component" -I"/home/tobias/eclipse-workspace/waveformgenerator2/SignalGeneration" -I"/home/tobias/eclipse-workspace/waveformgenerator2/System" -I"/home/tobias/eclipse-workspace/waveformgenerator2/UserInterface" -Os -Wall -Wextra -fmessage-length=0 -ffunction-sections -fdata-sections -c -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


