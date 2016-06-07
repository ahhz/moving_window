###############################################################################
#
# Require 3.0 for INTERFACE projects

cmake_minimum_required (VERSION 3.0)

###############################################################################
#
# Find header files and put in separate target for IDE
#

file(GLOB_RECURSE MY_HDRS "*.h")
add_custom_target(moving_window_headers SOURCES ${MY_HDRS})

###############################################################################
#
# Create an interface library for the header-only library
#

add_library(moving_window INTERFACE)
target_include_directories(moving_window INTERFACE "include/")
target_link_libraries(moving_window INTERFACE raster)
target_link_libraries(moving_window INTERFACE iterator)
target_link_libraries(moving_window INTERFACE boost_interface)

###############################################################################
#
# Create an executable using the header only interface
#
add_executable(moving_window_demo "examples/motivating_example.cpp")
target_link_libraries(moving_window_demo PRIVATE moving_window)

###############################################################################