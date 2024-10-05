cmake_minimum_required(VERSION 3.14)

message(STATUS "External source dependancies.")

# Multi-Platform Gamepad Library.
# add_subdirectory("MPG")

# Move the targets into a solution folder.
# set_property(TARGET PROPERTY FOLDER "MPG")

# Add an include directive for the header files.
# target_include_directories(centre_module PUBLIC ${CMAKE_CURRENT_LIST_DIR}/MPG/src)


# TOML Plus Plus
add_subdirectory("tomlplusplus")

# Move the targets into a solution folder.
set_property(TARGET PROPERTY FOLDER "tomlplusplus")

# Add an include directive for the header files.
target_include_directories(centre_module PUBLIC ${CMAKE_CURRENT_LIST_DIR}/tomlplusplus)
