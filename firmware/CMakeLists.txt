cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(gvobu-firmware C CXX)

pico_sdk_init()

add_executable(program
    main.cpp
    usb_descriptors.c
)

target_compile_features(program PRIVATE cxx_std_17)
target_include_directories(program PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(program
    pico_stdlib
        pico_multicore
    tinyusb_device
    tinyusb_board
    hardware_spi
    hardware_uart
        hardware_pwm
)

#pico_enable_stdio_usb(program 1)
pico_enable_stdio_uart(program 0)

pico_add_extra_outputs(program)

set(PICO_MOUNT_PATH /media/$ENV{USER}/RPI-RP2)

add_custom_target(flash
        COMMAND ${CMAKE_SOURCE_DIR}/flash_rp2350.sh
        $<TARGET_FILE_DIR:program>/program.uf2
        DEPENDS program.uf2
        COMMENT "Flashing UF2"
)
