# AT32 Control Panel

Firmware for the AT32A403AVGT7 controller installed on the test-bench control/measurement PCB.

The project is intended for Visual Studio 2022 + VisualGDB and uses the official Artery AT32A403A Firmware Library.

Initial scope:
- ADC1 + DMA acquisition of the 15 measurement channels from the bench schematic;
- conversion of raw ADC data to torque/current/voltage/temperature values;
- Nextion NX8048K070 communication over USART3 on PD8/PD9;
- J-Link/SWD debug and flash configuration for AT32A403AVGT7.

Run `setup_bsp.ps1` once after cloning to fetch the official Artery BSP.
