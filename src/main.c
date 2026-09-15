#include "at32a403a_conf.h"
#include "board_clock.h"
#include "bench_monitor.h"
#include "nextion.h"
#include "timebase.h"

extern bench_measurements_t g_measurements;

int main(void)
{
    system_clock_config();
    timebase_init();
    bench_adc_init();
    bench_rpm_init();
    nextion_init(9600U);

    delay_ms(800U);
    nextion_command("page 0");
    nextion_set_text("tStatus", "AT32 online");

    while (1) {
        bench_measurements_read(&g_measurements);
        nextion_show_measurements(&g_measurements);
        delay_ms(500U);
    }
}
