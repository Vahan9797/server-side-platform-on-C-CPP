cmake_minimum_required(VERSION 2.6)
project(SSP_On_C)

# The version numbers
set (SSP_On_C_VERSION_MAJOR, 1)
set (SSP_On_C_VERSION_MINOR, 0)

# Include directories
include_directories("./headers")

# Add the executable
add_executable(SSP_On_C ./server.c)