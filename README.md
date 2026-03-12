# TM4C123GH6PM_FreeRTOS
Lerning FreeRTOS 
## Environment
#### No IDE is used in this project. Everything is done from the command line on Linux.
**Hardware:** Texas Instruments TM4C123GH6PM (Tiva C Series Launchpad) — Cortex-M4F running at 16 MHz.

**Compiler:** arm-none-eabi-gcc — the standard GCC cross-compiler toolchain for bare-metal ARM targets. Compiles to Thumb-2 with hardware floating point (-mfpu=fpv4-sp-d16 -mfloat-abi=hard).

**Build system:** GNU Make. Running make build compiles all sources and links them into a .bin binary. The linker script (ld/tm4c123.ld) and a custom CMSIS-style startup file (startup_tm4c123.c) handle memory layout and the vector table manually — no vendor SDK or HAL is used.

**Flashing:** lm4flash — a lightweight command-line tool that flashes the binary to the board over USB via the onboard ICDI debugger. Run with make flash.

**Debugging:** openocd + arm-none-eabi-gdb for debug sessions when needed (make gdb).
**RTOS:** FreeRTOS kernel source included directly in the repository, built as part of the project rather than as a pre-compiled library.
