# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

A Raspberry Pi Pico (RP2040) driver for addressable LED strings (WS2812 / SK8612-style, 24-bit colour) written in C++. It uses the PIO block to generate the bit-timed serial protocol and DMA to feed the PIO FIFO, so the CPU is free after `Update()` is called (transfer completion is not signalled back to the caller — see below).

## Build

This is a Pico SDK / CMake project, built with the standard Pico toolchain (arm-none-eabi-gcc + Ninja/Make).

```bash
source Set_Environement.sh   # sets PICO_SDK_PATH (local machine path, not committed — see .gitignore)
mkdir -p build && cd build
cmake -G Ninja ..
ninja
```

- Output artifacts for the example firmware: `build/example_8_x_150.uf2` (flash via BOOTSEL mass-storage copy) and `.elf` (for debug probes).
- There is no test suite — this targets embedded hardware (Pico) and is verified by flashing and observing LED strips.
- `pico_sdk_import.cmake` and `Set_Environement.sh` are gitignored (machine-specific `PICO_SDK_PATH`); they must exist locally for the build to work.

## Architecture

Two build targets are defined in `CMakeLists.txt`:

- **`addressable_led`** (static library): `addressable_led.cpp`/`.hpp` + `ws2812.pio`. This is the reusable driver.
- **`example_8_x_150`** (executable): `example_8_x_150.cpp`, demonstrates driving 8 independent LED strips at once.

### PIO + DMA pipeline

- `ws2812.pio` defines the `ws2812` PIO program (bit-banged WS2812 timing via side-set) and a C-SDK helper `ws2812_program_init()` that configures a state machine's clock divider/shift direction for a given data rate and pin. CMake auto-generates `ws2812.pio.h` from this file at build time (`pico_generate_pio_header`, output into `build/`).
- Each `Addressable_LED` instance owns one PIO state machine (0-3) on one PIO block (`pio0`/`pio1`) and one DMA channel. The PIO program itself is only loaded into a given PIO block once — file-scope static flags `PIO_0_Initialised`/`PIO_1_Initialised` and `PIO_0_offset`/`PIO_1_offset` in `addressable_led.hpp` track this across all instances, since up to 4 state machines share one PIO block/program.
- Colour data lives in a `uint32_t LED_Data[]` buffer per strip (one word per LED, top 24 bits used, GRB or RGB byte order depending on which `Colour`/`Colour_RGB` enum is used — see below). `Update()` configures and kicks off a DMA transfer from this buffer straight into the PIO TX FIFO (`pioX_hw->txf[sm]`), gated by the state-machine-specific `DREQ_PIOx_TXn` data request signal, and returns immediately without blocking.
- **Caller responsibility**: because `Update()` doesn't block or signal completion, the caller must ensure a transfer has finished (e.g. by pacing updates on a period longer than the transmit time) before mutating `LED_Data` again or calling `Update()` again on the same strip. This is a documented constraint, not something the driver enforces.
- Up to 8 strips can run concurrently: 4 state machines × 2 PIO blocks, wired to the 8 PCB header connectors `J1`-`J8` (`Addressable_LED::Channels` enum), which map to fixed Pico GPIO pins (15 down to 8).

### Colour byte order gotcha

There are two colour enums with the *same byte values in different positions*, and picking the wrong one for how the strip is wired will swap channels:
- `Colour` (`Red`, `Yellow`, ... ) — byte order **GRB**.
- `Colour_RGB` (`Red_RGB`, `Yellow_RGB`, ...) — byte order **RGB**.

Which one is correct depends on the physical LED string's expected byte order, not on the code — check against the actual hardware before assuming.

### LED_Data manipulation API

`Rainbow_Serpent`, `Fill`, `Solid`, `Set_One` all mutate the `LED_Data` buffer in-place and are cheap/CPU-side; none of them touch PIO/DMA. Only `Update()` touches hardware. This split means it's safe to call the mutator methods repeatedly while a previous DMA transfer is still draining, as long as `Update()` itself isn't re-issued until the prior transfer completes (see caller responsibility above).
