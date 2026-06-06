find_package(SDL2 REQUIRED)

add_library(display_port_linux STATIC
    native_sim_display_port.c
)

target_include_directories(display_port_linux
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(display_port_linux
    PUBLIC
        portable_display_driver
        ${SDL2_LIBRARIES}
)

target_compile_options(display_port_linux PRIVATE -Wno-pedantic)
