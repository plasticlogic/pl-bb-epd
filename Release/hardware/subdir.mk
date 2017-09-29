################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hardware/dac-max5820-hv.c \
../hardware/dac-max5820.c \
../hardware/nvm-i2c-24AA256.c \
../hardware/nvm-i2c-24LC014H.c \
../hardware/nvm-spi-MX25U4033E.c \
../hardware/pmic-max17135-hv.c \
../hardware/pmic-max17135.c \
../hardware/pmic-tps65185-hv.c \
../hardware/pmic-tps65185.c 

OBJS += \
./hardware/dac-max5820-hv.o \
./hardware/dac-max5820.o \
./hardware/nvm-i2c-24AA256.o \
./hardware/nvm-i2c-24LC014H.o \
./hardware/nvm-spi-MX25U4033E.o \
./hardware/pmic-max17135-hv.o \
./hardware/pmic-max17135.o \
./hardware/pmic-tps65185-hv.o \
./hardware/pmic-tps65185.o 

C_DEPS += \
./hardware/dac-max5820-hv.d \
./hardware/dac-max5820.d \
./hardware/nvm-i2c-24AA256.d \
./hardware/nvm-i2c-24LC014H.d \
./hardware/nvm-spi-MX25U4033E.d \
./hardware/pmic-max17135-hv.d \
./hardware/pmic-max17135.d \
./hardware/pmic-tps65185-hv.d \
./hardware/pmic-tps65185.d 


# Each subdirectory must supply rules for building sources it contributes
hardware/%.o: ../hardware/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\libpng-1.2.51" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\pl" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


