#ifndef __AT32A403A_CONF_H
#define __AT32A403A_CONF_H
#ifdef __cplusplus
extern "C" {
#endif
#ifndef HEXT_VALUE
#define HEXT_VALUE ((uint32_t)8000000U)
#endif
#define HEXT_STARTUP_TIMEOUT ((uint16_t)0x3000U)
#define HICK_VALUE ((uint32_t)8000000U)
#define LEXT_VALUE ((uint32_t)32768U)
#define CRM_MODULE_ENABLED
#define GPIO_MODULE_ENABLED
#define USART_MODULE_ENABLED
#define ADC_MODULE_ENABLED
#define DMA_MODULE_ENABLED
#define TMR_MODULE_ENABLED
#define FLASH_MODULE_ENABLED
#define MISC_MODULE_ENABLED
#include "at32a403a_crm.h"
#include "at32a403a_gpio.h"
#include "at32a403a_usart.h"
#include "at32a403a_adc.h"
#include "at32a403a_dma.h"
#include "at32a403a_tmr.h"
#include "at32a403a_flash.h"
#include "at32a403a_misc.h"
#ifdef __cplusplus
}
#endif
#endif
