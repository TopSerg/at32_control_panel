#include "timebase.h"
#include "at32a403a.h"

static volatile uint32_t g_ms = 0U;

void timebase_init(void)
{
    g_ms = 0U;
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000U);
}

uint32_t timebase_millis(void) { return g_ms; }

void delay_ms(uint32_t ms)
{
    const uint32_t start = g_ms;
    while ((uint32_t)(g_ms - start) < ms) { __NOP(); }
}

void SysTick_Handler(void) { ++g_ms; }
