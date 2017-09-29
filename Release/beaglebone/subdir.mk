################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../beaglebone/beaglebone-gpio.c \
../beaglebone/beaglebone-hv.c \
../beaglebone/beaglebone-i2c.c \
../beaglebone/beaglebone-spi.c 

OBJS += \
./beaglebone/beaglebone-gpio.o \
./beaglebone/beaglebone-hv.o \
./beaglebone/beaglebone-i2c.o \
./beaglebone/beaglebone-spi.o 

C_DEPS += \
./beaglebone/beaglebone-gpio.d \
./beaglebone/beaglebone-hv.d \
./beaglebone/beaglebone-i2c.d \
./beaglebone/beaglebone-spi.d 


# Each subdirectory must supply rules for building sources it contributes
beaglebone/%.o: ../beaglebone/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\libpng-1.2.51" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\pl" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


