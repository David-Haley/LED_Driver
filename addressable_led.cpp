// Implementation of Addressable_LED class
//
// Class to manage addressable LED strips using the protocol used by
// WS2812 addressable LEDs or similar.
// Author    : David Haley
// Created   : 23/10/2021
// Last Edit : 22/10/2023
// 20231022: Provide for DMA transfer to PIO.
// 20220723: Black, White and Set_One added.
// 20211026: One shot initialisation of PIOs implemented, Step_Size
// and Solid added.

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "ws2812.pio.h"
#include "addressable_led.hpp"

Addressable_LED :: Addressable_LED (
        uint Number_of_LEDs,
        PIO PIO_to_Use, // pio0 or pio1
        int State_Machine_to_Use, // 0, 1, 2 or 3
        Channels Channel)

{
	LED_Count = Number_of_LEDs;
	LED_Data = new uint32_t [LED_Count];
	for (uint I = 0; I < LED_Count; I++)
		LED_Data [I] = 0;
	pio = PIO_to_Use;
	sm = State_Machine_to_Use;
	Tx_Pin = Channel;
	const float Bit_Rate =800000.0; // 800 kHz
        DMA_Channel = dma_claim_unused_channel (true); // Panic if none free
        DMA_Config =  dma_channel_get_default_config (DMA_Channel);
        dma_channel_get_default_config (DMA_Config, DMA_Size_32);
        channel_config_set_read_increment (DMA_Config, true);
        // step through the blok of LED_Data;
        channel_config_set_write_increment (DMA_Config, false);
        // Destination is PIO FIFO, that is, a fixed address
	if (pio == pio0)
	{ 
		if (! PIO_0_Initialised)
		{
			PIO_0_Initialised = true;
		    PIO_0_offset = pio_add_program (pio, &ws2812_program);
		} // if (! PIO_0_Initialised)
                ws2812_program_init (pio, sm, PIO_0_offset, Tx_Pin, Bit_Rate,
                  false); // RGB data only, no white LEDs
                switch (State_Machine_to_Use)
                {
                        case 0:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO0_TX0);
                                break;
                        case 1:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO0_TX1);
                                break;
                        case 2:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO0_TX2);
                                break;
                        case 3:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO0_TX3;
                                break;
                } // switch (State_Machine_to_Use)
	} // if (pio == pio0)
	else if(pio == pio1)
	{
		if (! PIO_1_Initialised)
		{
			PIO_1_Initialised = true;
		    PIO_1_offset = pio_add_program (pio, &ws2812_program);
		} // (! PIO_1_Initialised)
                ws2812_program_init (pio, sm, PIO_1_offset, Tx_Pin, Bit_Rate,
                  false); // RGB data only, no white LEDs
                switch (State_Machine_to_Use)
                {
                        case 0:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO1_TX0);
                                break;
                        case 1:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO1_TX1);
                                break;
                        case 2:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO1_TX2);
                                break;
                        case 3:
                                channel_config_set_dreq	(DMA_Config,
                                  DREQ_PIO1_TX3;
                                break;
                } // switch (State_Machine_to_Use)
	} // else if(pio == pio1)
} // Addressable_LED

void Addressable_LED :: Rainbow_Serpent (bool Initialise, bool Forward,
	uint Step_Size)

{
    const uint32_t Colour_Table [Colours] =
		{Red, Yellow, Green, Cyan, Blue, Magenta};
	if (Initialise)
	{
		for (uint I = 0; I < LED_Count; I++)
		{
			LED_Data [I] = Colour_Table [(I * Colours) / LED_Count];
			// for all I < LED_Count 
			// 0 <= Colour_Table Index <= Colours - 1
			// This ensures as even as possible distribution of colours
			// when LED_Count % Colours != 0
		} // for (uint I = 0; I < LED_Count; I++)
	} // if (Initialise)
	else
	{ // Run
		for (uint S = 1; S <= Step_Size; S++)
		{
			if (Forward)
			{ // Forward
				uint32_t Temp = LED_Data [LED_Count - 1];
				for (uint I = LED_Count - 1; I > 0; I--)
					LED_Data [I] = LED_Data [I - 1];
				LED_Data [0] = Temp;
			} // Forward
			else
			{ // Reverse
				uint32_t Temp = LED_Data [0];
				for (uint I = 0; I < LED_Count - 1; I++)
					LED_Data [I] = LED_Data [I + 1];
				LED_Data [LED_Count - 1] = Temp;
			} // Reverse
		} // for (uint S = 1; S <= Step_Size; S++)
	} // Run
} // Rainbow_Serpent

void Addressable_LED :: Fill (uint32_t Colour, bool Forward,
	uint Step_Size)

{
	for (uint S = 1; S <= Step_Size; S++)
	{
		if (Forward)
		{ // Forward
			for (uint I = LED_Count - 1; I > 0; I--)
				LED_Data [I] = LED_Data [I - 1];
			LED_Data [0] = Colour;
		} // Forward
		else
		{ // Reverse
			for (uint I = 0; I < LED_Count - 1; I++)
				LED_Data [I] = LED_Data [I + 1];
			LED_Data [LED_Count - 1] = Colour;
		} // Reverse
	} // for (uint S = 1; S <= Step_Size; S++)
} // Fill

void Addressable_LED :: Solid (uint32_t Colour)
// Instant fill with one colour

{
	for (uint I = 0; I < LED_Count; I++)
		LED_Data [I] = Colour;
} // Solid

void Addressable_LED :: Set_One  (uint32_t Colour, uint LED_Number)
// Sets the LED in position LED_Number to Colour

{
        LED_Data [LED_Number] = Colour;
} // Set_One

void Addressable_LED :: Update (void)
// Send the stored LED data to the LED strips via DMA transfer, should return
// almost immedistely.

{
	if (pio == pio0)
	{
                dma_channel_configure (
                        DMA_Channel,
                        DMA_Config,
                        pio0_hw->txf[State_Machine_to_Use], // write address
                        LED_Data, // read address
                        true); // start immediately
        } // if (pio == pio0)
	else if (pio == pio1)
	{
                dma_channel_configure (
                        DMA_Channel,
                        DMA_Config,
                        pio1_hw->txf[State_Machine_to_Use], // write address
                        LED_Data, // read address
                        true); // start immediately
        } // if (pio == pio1)
} // Update
