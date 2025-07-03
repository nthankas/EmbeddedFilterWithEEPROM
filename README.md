Author: Nikhil Thankasala

Overview:  
This project implements a mixed-signal signal processing system on a PIC32 microcontroller. The system samples external analog inputs using the ADC, filters them in real time using software-implemented FIR low-pass or high-pass filters, and outputs the results through a digital-to-analog converter (DAC). The filter coefficients are stored in external EEPROM and can be dynamically updated through a serial protocol. The system is configurable through physical switch inputs and communicates with a host PC using a custom packet-based UART protocol.

Architecture and Design:  
The software stack is modular and built around three key drivers:

- **I2C & EEPROM (NVM)**: A custom I2C driver interfaces with an external 27LC256 EEPROM to store and retrieve filter coefficients. The EEPROM supports byte-level and page-level access with write-complete polling. A protocol-compliant test harness supports read/write validation and persistence across power cycles.

- **ADC + Digital Filter Engine**: The ADC samples four analog channels in scan mode and feeds the data into a real-time digital filter engine. A fixed-size circular buffer maintains the latest samples, and FIR filtering is performed using 32-tap integer-weighted convolution. Coefficients are loaded from EEPROM and are selectable between high-pass and low-pass filter banks.

- **DAC Output via SPI**: The filtered output is written to an external MCP4911 DAC via SPI. Output updates are driven by an internal timer synced to ADC conversion completion to maintain consistent throughput.

User input is provided via 4 physical switches:
- SW1 + SW2: Select one of 4 ADC channels
- SW3: Toggle between filter A and filter B
- SW4: Choose between absolute value or peak-to-peak LED display mode

Every 10ms, the system checks for switch changes and transmits updated ADC values (both raw and filtered) to the host. A bar graph representation of the filtered output is displayed on onboard LEDs, depending on the selected mode.

Development Process:  
The project began with building a low-level I2C communication layer and layering EEPROM protocol transactions on top. Care was taken to correctly handle page alignment, buffer wraparound, and read-after-write polling. The ADC engine was configured for auto-sample and scan mode to acquire all channels in the background. The ISR collects and forwards data into a per-channel FIFO structure.

Filter logic was implemented as a signed integer convolution with selectable filter banks. Endian conversion and integer scaling were necessary due to differing byte order between the PIC32 and the host-side Python console. Extensive testing was performed using sine wave inputs at known frequencies to validate filter behavior and dynamic range.

Outcome:  
The final embedded system performs responsive, real-time analog signal processing with flexible configuration through hardware and software. It demonstrates key principles of digital filtering, mixed-signal I/O, peripheral bus management, and EEPROM-based state persistence. The modular architecture allows the system to be extended with new filter types, input devices, or output visualizations with minimal structural changes. This project deepened my understanding of driver abstraction, ISR-safe processing, integer DSP, and protocol-driven host communication. Approximate development time: 20–22 hours.
