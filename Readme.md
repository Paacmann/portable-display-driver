# Portable Display Driver 

A lightweight and portable embedded display driver written in C. 

The project is designed around a hardware abstraction layer (HAL) style architecture where low-level communication (I2C, SPI , ...) is abstracted through function pointers.
This allows the core driver to remain platform-independent and easily portable across different microcontrollers and frameworks.

