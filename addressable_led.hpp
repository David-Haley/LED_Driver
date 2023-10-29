// Declaration of Addressable_LED class
//
// Class to manage addressable LED strips using the protocol used by
// WS2812 addressable LEDs or similar.
// Author    : David Haley
// Created   : 23/10/2021
// Last Edit : 22/10/2023
// 20231022: Provide for DMA transfer to PIO.
// 20221126: Black and White now static
// 20220723: Black, White and Set_One added.
// 20220719: Colour_RGB defined to match WS2811 LED driver.
// 20211026: One shot initialisation of PIOs implemented, Step_Size
// and Solid added.

#include "hardware/pio.h"
#include "hardware/dma.h"

static bool PIO_0_Initialised, PIO_1_Initialised = false;
static uint PIO_0_offset, PIO_1_offset;

class Addressable_LED
{
private:

int LED_Count = 0; // Number of LEDs in the strip
uint32_t *LED_Data = 0; // pointer to array of LED data
PIO pio; // PIO to be used
int sm; // state machine within PIO
int Tx_Pin;
uint DMA_Channel; // DMA chanel to be assigned to this string driver.
dma_channel_config DMA_Config;
// Configeration of the DMA channel assigned to this string driver. 

public:

enum Channels {J1 = 15, J2 = 14, J3 = 13, J4 = 12,
	J5 = 11, J6 = 10, J7 = 9, J8 = 8}; // PCB connectors
        
static const uint32_t Black = 0;
static const uint32_t White = 0x55555500;
enum Colour 
	{ //                 G R B
		Red =     0x00FE0000,
		Yellow =  0x7F7F0000,
		Green =   0xFE000000,
		Cyan =    0x7F007F00,
		Blue =    0x0000FE00,
		Magenta = 0x007F7F00
	}; // Colour
enum Colour_RGB 
	{ //                     R G B
		Red_RGB =     0xFE000000,
		Yellow_RGB =  0x7F7F0000,
		Green_RGB =   0x00FE0000,
		Cyan_RGB =    0x007F7F00,
		Blue_RGB =    0x0000FE00,
		Magenta_RGB = 0x7F007F00
	}; // Colour_RGB
static const uint Colours = 6;

Addressable_LED (
	uint Number_of_LEDs,
	PIO PIO_to_Use, // pio0 or pio1
	int State_Machine_to_Use, // 0, 1, 2 or 3
	Channels Channel);
	
void Rainbow_Serpent (bool Initialise, bool Forward,
	uint Step_Size = 1);
// Moving blocks of colour not valid for Colour_RGB

void Fill (uint32_t Colour, bool Forward, uint Step_Size = 1);
// Progressively fill with one colour

void Solid (uint32_t Colour);
// Instant fill with one colour

void Set_One (uint32_t Colour, uint LED_Number);
// Sets the LED in position LED_Number to Colour

void Update (void);
// Send the stored LED data to the LED strips via DMA transfer, should return
// almost immedistely.

}; // Addressable_LED
