add_library(display_port_stm32f1 STATIC
    stm32f1x_display_port.c
)

add_library(display_port_stm32_f1 ALIAS display_port_stm32f1)

target_include_directories(display_port_stm32f1
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(display_port_stm32f1
    PUBLIC
        portable_display_driver
    PRIVATE
        # stm32_hal
)




# add_library(stm32_hal INTERFACE)

# target_include_directories(stm32_hal INTERFACE
#    Drivers/STM32F1xx_HAL_Driver/Inc
#    Drivers/CMSIS/Include
#    Drivers/CMSIS/Device/ST/STM32F1xx/Include
#    Core/Inc
# )

# target_compile_definitions(stm32_hal INTERFACE
#    USE_HAL_DRIVER
#    STM32F103xB
# )

