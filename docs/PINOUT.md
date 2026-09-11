# Pinout used by the AT32 control panel

## ADC1 + DMA

| Signal | Pin | ADC |
|---|---|---:|
| Torque | PA0 | IN0 |
| Phase current A | PA1 | IN1 |
| Phase current B | PA2 | IN2 |
| Phase current C | PA3 | IN3 |
| DC current 1 | PA4 | IN4 |
| DC current 2 | PA5 | IN5 |
| Board temperature | PA6 | IN6 |
| External temperature 1 | PB0 | IN8 |
| External temperature 2 | PB1 | IN9 |
| +12 V monitor | PC0 | IN10 |
| Phase voltage A | PC1 | IN11 |
| Phase voltage B | PC2 | IN12 |
| Phase voltage C | PC3 | IN13 |
| DC voltage 1 | PC4 | IN14 |
| DC voltage 2 | PC5 | IN15 |

PB12/PB13 enable external-temperature pull-ups.

## RPM
PB5 = TMR3_CH2, remap TMR3_GMUX_0010. Initial setting: 60 pulses/revolution.

## Nextion
PD8 = USART3_TX, PD9 = USART3_RX, remap USART3_GMUX_0011.

## Reserved interfaces
PA9/PA10 = USART1 AT32<->ESP32; PA11/PA12 = CAN1; PD5/PD6 = USART2 RS-485; PD4 = RS-485 direction.

## SWD X1
X1.13 SWDIO/PA13; X1.11 SWCLK/PA14; X1.9 NRST; X1.10 SWO/PB3; X1.1/14 3.3V; X1.3/7/8/12 GND.
