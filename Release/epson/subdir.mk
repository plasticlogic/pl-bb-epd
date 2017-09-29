################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../epson/epson-i2c.c \
../epson/epson-s1d135xx.c \
../epson/epson-spi.c \
../epson/s1d13524_controller.c \
../epson/s1d13541-nvm.c \
../epson/s1d13541_controller.c \
../epson/s1d135xx_hv.c 

OBJS += \
./epson/epson-i2c.o \
./epson/epson-s1d135xx.o \
./epson/epson-spi.o \
./epson/s1d13524_controller.o \
./epson/s1d13541-nvm.o \
./epson/s1d13541_controller.o \
./epson/s1d135xx_hv.o 

C_DEPS += \
./epson/epson-i2c.d \
./epson/epson-s1d135xx.d \
./epson/epson-spi.d \
./epson/s1d13524_controller.d \
./epson/s1d13541-nvm.d \
./epson/s1d13541_controller.d \
./epson/s1d135xx_hv.d 


# Each subdirectory must supply rules for building sources it contributes
epson/%.o: ../epson/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\libpng-1.2.51" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\pl" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


