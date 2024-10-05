# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(control_panels-64 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Check if the submodules are present, download if needed.
find_package(Git QUIET)
if(GIT_FOUND)
        # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

set(MAIN_PROJECT_PATH ${PROJECT_SOURCE_DIR})
set(EXTERNAL_PATH ${PROJECT_SOURCE_DIR}/src/external)

# Hardware folders:
add_subdirectory(centre-module)

# Add any external dependancies.
add_subdirectory("${EXTERNAL_PATH}")

# # Safety check - just check one of the submodules we know should be there now.
# if(NOT EXISTS "${EXTERNAL_PATH}/MPG/CMakeLists.txt")
#     message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
# endif()

# # Add TOML++ as a dependancy.
# include(FetchContent)
# FetchContent_Declare(
#     tomlplusplus
#     GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
#     GIT_TAG        v3.1.0
# )
# FetchContent_MakeAvailable(tomlplusplus)

# target_include_directories(centre_module PUBLIC ${tomlplusplus_SOURCE_DIR}/include/toml++)

# Add an include directive for the header files.
target_include_directories(centre_module PUBLIC ${CMAKE_CURRENT_LIST_DIR}/MPG/src)
