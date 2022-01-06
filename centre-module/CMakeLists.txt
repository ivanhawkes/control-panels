cmake_minimum_required(VERSION 3.14)

add_executable(centre_module)

target_sources(centre_module PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/Main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/TinyUSBDescriptors.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/TinyUSBCallbacks.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/DigitalInput.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/AnalogueInput.cpp
        )

# Set a path to our includes.
target_include_directories(centre_module PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        ${EXTERNAL_PATH}/entt/src)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support, and tinyusb_board for the additional board support library.
target_link_libraries(centre_module PUBLIC pico_stdlib hardware_adc
        tinyusb_device tinyusb_board
        pico_bootsel_via_double_reset)

# This allows us to rename the tusb_config.h file into something a little more in line with naming convention.
add_definitions(-DCFG_TUSB_CONFIG_FILE="${CMAKE_CURRENT_LIST_DIR}/include/TinyUSBConfig.h") 

pico_add_extra_outputs(centre_module)

# Enable USB *or* UART output.
pico_enable_stdio_usb(centre_module 0)
pico_enable_stdio_uart(centre_module 1)
