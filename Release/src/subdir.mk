################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BBepdcULD.c \
../src/configparser.c \
../src/dictionary.c \
../src/hw_setup.c \
../src/iniparser.c 

OBJS += \
./src/BBepdcULD.o \
./src/configparser.o \
./src/dictionary.o \
./src/hw_setup.o \
./src/iniparser.o 

C_DEPS += \
./src/BBepdcULD.d \
./src/configparser.d \
./src/dictionary.d \
./src/hw_setup.d \
./src/iniparser.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\libpng-1.2.51" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\pl" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


