cmake_minimum_required(VERSION 3.16)

project(generic_display_driver LANGUAGES C)

set(PROJECT_VERSION "1.0.0")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


option(DISPLAY_BUILD_STM32F1_PORT "Build STM32F1 display port" OFF)
option(DISPLAY_BUILD_LINUX_SIM "Build SDL2 native simulator port" OFF)


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


# Native linux port
#--------------------------
# find_package(SDL2 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(SDL2 REQUIRED sdl2)

if (DISPLAY_BUILD_LINUX_SIM)

    add_subdirectory(port/native_sim)
    
    add_executable(sim_test
        examples/sim_test/main.c
    )
    
    target_include_directories(sim_test PRIVATE ${SDL2_INCLUDE_DIRS})


    target_link_libraries(sim_test
        portable_display_driver
        display_port_linux
        ${SDL2_LIBRARIES}
    )

endif()
#-------------------------

# STM32F1 port (optional)

if (DISPLAY_BUILD_STM32F1_PORT)
    add_subdirectory(port/stm32/f1)
endif()

#--------------------------




