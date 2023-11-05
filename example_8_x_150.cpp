// Example of eight strips being filled with different data and using the DMA
// version of the LED Driver.
// Author    : David Haley
// Created   : 05/11/2023
// Last Edit : 05/11/2023

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "hardware/pio.h"
#include "addressable_led.hpp"

const uint64_t Update = 100000; // 100 ms
const uint Colours = 6;
const uint Monitor = 7; // Monitor pin high when CPU is setting up LED data
const uint Strip_Length = 150;

int main ()

{
	stdio_init_all();
	gpio_init (Monitor);
	gpio_set_dir (Monitor, GPIO_OUT);
	gpio_init (PICO_DEFAULT_LED_PIN);
	gpio_set_dir (PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put (PICO_DEFAULT_LED_PIN, 1);
	watchdog_enable(1500, true); // 1.5 s and allow debug
        // Declare LED strips
	Addressable_LED Strip_1 (Strip_Length, pio0, 0, Addressable_LED :: J1);
	Addressable_LED Strip_2 (Strip_Length, pio0, 1, Addressable_LED :: J2);
	Addressable_LED Strip_3 (Strip_Length, pio0, 2, Addressable_LED :: J3);
	Addressable_LED Strip_4 (Strip_Length, pio0, 3, Addressable_LED :: J4);
	Addressable_LED Strip_5 (Strip_Length, pio1, 0, Addressable_LED :: J5);
	Addressable_LED Strip_6 (Strip_Length, pio1, 1, Addressable_LED :: J6);
	Addressable_LED Strip_7 (Strip_Length, pio1, 2, Addressable_LED :: J7);
	Addressable_LED Strip_8 (Strip_Length, pio1, 3, Addressable_LED :: J8);
	absolute_time_t Next_Time = get_absolute_time ();
	uint32_t Step_Count = 0;
	while (true)
	{
		Next_Time = delayed_by_us (Next_Time, Update);
		busy_wait_until (Next_Time);
		gpio_put (Monitor, 1);
                bool Initialise = Step_Count % Strip_Length == 0;
                if (Step_Count % 10 < 5)
			gpio_put (PICO_DEFAULT_LED_PIN, 0);
		else
			gpio_put (PICO_DEFAULT_LED_PIN, 1);

                Strip_1.Rainbow_Serpent (Step_Count == 0, true);
                // Rainbow running forward
                if (Initialise)
                {
                        Strip_2.Solid (Strip_2.Black);
                        Strip_3.Solid (Strip_3.Black);
                        Strip_4.Solid (Strip_4.Black);
                        Strip_5.Solid (Strip_5.Black);
                        Strip_6.Solid (Strip_6.Black);
                        Strip_7.Solid (Strip_7.Black);
                } //if (Initialise)
                Strip_2.Fill (Strip_2.Red, true); // fill forward with Red
                Strip_3.Fill (Strip_3.Yellow, true); // fill forward with Yellow
                Strip_4.Fill (Strip_4.Green, true); // fill forward with Green
                Strip_5.Fill (Strip_5.Cyan, true); // fill forward with Cyan
                Strip_6.Fill (Strip_6.Blue, true); // fill forward with Blue
                Strip_7.Fill (Strip_7.Magenta, true);
                // fill forward with Magenta
                Strip_8.Rainbow_Serpent (Step_Count == 0, false);
                // Rainbow running backwards
                Strip_1.Update ();
                Strip_2.Update ();
                Strip_3.Update ();
                Strip_4.Update ();
                Strip_5.Update ();
                Strip_6.Update ();
                Strip_7.Update ();
                Strip_8.Update ();
                Step_Count++;
		watchdog_update();
		gpio_put (Monitor, 0);
	} // while (true)
} // main
