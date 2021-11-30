cmake_minimum_required(VERSION 3.13)

add_executable(centre_module)

target_sources(centre_module PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(centre_module PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(centre_module PUBLIC pico_stdlib tinyusb_device tinyusb_board)

pico_add_extra_outputs(centre_module)

# Enable USB output, disable UART output.
#pico_enable_stdio_usb(centre_module 1)
#pico_enable_stdio_uart(centre_module 0)

# add url via pico_set_program_url
#example_auto_set_url(centre_module)
