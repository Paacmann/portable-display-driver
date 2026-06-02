add_library(portable_display_driver STATIC 
    display.c
)

target_include_directories(portable_display_driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

