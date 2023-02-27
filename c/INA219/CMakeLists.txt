cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(INA219_project)

# Initialize the SDK
pico_sdk_init()

add_executable(INA219
        INA219.c
)
            
# pull in common dependencies
target_link_libraries(INA219 hardware_i2c hardware_spi hardware_pwm pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(INA219 1)
pico_enable_stdio_uart(INA219 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(INA219)

