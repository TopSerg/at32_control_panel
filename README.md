# AT32 Control Panel

Отдельная прошивка панели стенда на **AT32A403AVGT7**. Репозиторий инвертора `controllerat32` не используется и не изменяется.

Проект сделан в том же формате, что и рабочий проект инвертора: **Visual Studio 2022 + VisualGDB + arm-none-eabi GCC + J-Link/SWD**.

## Уже реализовано

- ADC1 + circular DMA для 15 аналоговых каналов платы;
- момент;
- 3 фазных тока и 2 DC-тока;
- 3 фазных напряжения и 2 DC-напряжения;
- контроль +12 V;
- температура платы KTY82/110;
- два внешних температурных входа пока показываются как напряжение: тип датчика в материалах стенда не указан;
- RPM на PB5 / TMR3_CH2;
- Nextion NX8048K070 по USART3, PD8/PD9;
- J-Link/SWD для AT32A403AVGT7.

## Первый запуск

Клонировать:

```powershell
git clone https://github.com/TopSerg/at32_control_panel.git
cd at32_control_panel
```

Один раз подготовить официальный BSP Artery:

```powershell
.\setup_bsp.ps1
```

Если AT32A403A Firmware Library уже скачана:

```powershell
.\setup_bsp.ps1 -BspRoot "D:\AT32IDE\repo\BSP\AT32A403A"
```

После этого появится локальная папка `vendor` с drivers/CMSIS/startup/linker. Она не коммитится.

Открыть:

```text
AT32_ControlPanel.sln
```

Выбрать `Debug | VisualGDB` и запускать F5. Debug-конфигурация использует:

```text
J-Link
-device AT32A403AVGT7
-if SWD
```

## SWD платы

| X1 | Сигнал |
|---:|---|
| 13 | SWDIO / PA13 |
| 11 | SWCLK / PA14 |
| 9 | NRST |
| 10 | SWO / PB3, необязательно |
| 1 / 14 | 3.3 V VTref |
| 3 / 7 / 8 / 12 | GND |

## Калибровка

Текущие коэффициенты являются начальными значениями из схемы стенда:

- torque: 0.0976 N*m/count, zero = 2048;
- currents: 0.258 A/count, zero = 2048;
- phase voltage: 0.0489 V/count, zero = 2048;
- DC voltage: 0.0404 V/count.

Перед точными измерениями нули и gain нужно откалибровать на физическом стенде.

## Nextion

Имена текстовых объектов страницы 0 см. в `docs/NEXTION_OBJECTS.md`. HMI/TFT-файла в переданных материалах не было, поэтому при наличии уже готового интерфейса имена может понадобиться поменять в `src/nextion.c`.

## Что пока намеренно не добавлено

- управление Sintech;
- CAN-протокол;
- обмен с ESP32;
- RS-485;
- кнопки/команды с Nextion.

Сначала лучше проверить базовый тракт: прошивка -> ADC -> RPM -> экран.
