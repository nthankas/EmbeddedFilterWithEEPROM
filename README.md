# Embedded Filter with EEPROM

Mixed-signal signal processing system on PIC32. Samples analog inputs via ADC, applies real-time FIR filtering (high-pass or low-pass), and outputs through an MCP4911 DAC over SPI. Filter coefficients are stored in external 27LC256 EEPROM via I2C and can be updated dynamically through a custom UART packet protocol.

## Architecture

- **ADC engine**: 4-channel scan mode with ISR-driven circular buffer. 32-tap FIR convolution with signed integer weights
- **I2C/EEPROM driver**: byte and page-level read/write to 27LC256 with write-complete polling. stores filter coefficient banks across power cycles
- **Protocol layer**: custom packet-based UART protocol (head/len/id/payload/tail/checksum/CR/LF) with iterative checksum, endianness conversion, and packet queue for async processing
- **DAC output**: MCP4911 driven via SPI, synced to ADC conversion rate

## Controls

- SW1 + SW2: select ADC channel (1 of 4)
- SW3: toggle between filter A and filter B
- SW4: absolute value vs peak-to-peak LED display mode

## Target

PIC32MX320F128H (Uno32). Build with MPLAB X -- open `FilterEEPROM.X/`.
