# LED_Driver
A driver for addressable LED strings (SK8612 or similar)  supporting 24 colour data data on Pi Pico.
Uses PIO and DMA to free up CPU time. Calls to Addressable_LED.Update should not block, however it is the programmer's responsibility to ensure that the transfer completes before updating the LEDs. If the strings are updated on a time basis, provided that this time is greater than the time required to transmit the data, this condition iss satisfied.
