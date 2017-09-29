################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ultrachip/uc8156.c \
../ultrachip/uc8156_controller.c \
../ultrachip/uc8156_hv.c \
../ultrachip/uc8156_nvm.c 

OBJS += \
./ultrachip/uc8156.o \
./ultrachip/uc8156_controller.o \
./ultrachip/uc8156_hv.o \
./ultrachip/uc8156_nvm.o 

C_DEPS += \
./ultrachip/uc8156.d \
./ultrachip/uc8156_controller.d \
./ultrachip/uc8156_hv.d \
./ultrachip/uc8156_nvm.d 


# Each subdirectory must supply rules for building sources it contributes
ultrachip/%.o: ../ultrachip/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\libpng-1.2.51" -I"C:\Users\robert.pohlink\workspace\ws_BB\BBepdcULD\pl" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


