# LED_Driver
A driver for addressable LED strings (SK8612 or similar)  supporting either 24 or 32 bit colour data data on Pi Pico.
Uses PIO and as a future development DMA to free up CPU time. Without DMA update calls for strings of 8 or fewer LEDs will not block.
