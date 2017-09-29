################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pl/display.c \
../pl/generic_controller.c \
../pl/generic_epdc.c \
../pl/gpio.c \
../pl/hv.c \
../pl/hwinfo.c \
../pl/i2c.c \
../pl/nvm.c \
../pl/parser.c \
../pl/scramble.c \
../pl/utils.c \
../pl/vcom.c 

OBJS += \
./pl/display.o \
./pl/generic_controller.o \
./pl/generic_epdc.o \
./pl/gpio.o \
./pl/hv.o \
./pl/hwinfo.o \
./pl/i2c.o \
./pl/nvm.o \
./pl/parser.o \
./pl/scramble.o \
./pl/utils.o \
./pl/vcom.o 

C_DEPS += \
./pl/display.d \
./pl/generic_controller.d \
./pl/generic_epdc.d \
./pl/gpio.d \
./pl/hv.d \
./pl/hwinfo.d \
./pl/i2c.d \
./pl/nvm.d \
./pl/parser.d \
./pl/scramble.d \
./pl/utils.d \
./pl/vcom.d 


# Each subdirectory must supply rules for building sources it contributes
pl/%.o: ../pl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\libpng-1.2.51" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\pl" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


