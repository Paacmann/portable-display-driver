cmake_minimum_required(VERSION 3.16)

project(generic_display_driver LANGUAGES C)

set(PROJECT_VERSION "1.0.0")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


option(DISPLAY_BUILD_STM32F1_PORT "Build STM32F1 display port" OFF)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wundef
    -Wdouble-promotion
    -fno-common
)

#---------------------------
# Core driver , always Build

add_subdirectory(driver)

#---------------------------



#--------------------------
# STM32F1 port (optional)

if (DISPLAY_BUILD_STM32F1_PORT)
    add_subdirectory(port/stm32/f1)
endif()

#---------------------------
