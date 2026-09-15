#include "bench_monitor.h"
#include "timebase.h"
#include "at32a403a_conf.h"
#include <stddef.h>

#define ADC_VREF_V            3.3f
#define ADC_FULL_SCALE        4095.0f

/* Initial nominal values from the bench schematic. Calibrate on real hardware. */
#define ADC_CENTER_COUNTS     2048.0f
#define TORQUE_NM_PER_COUNT   0.0976f
#define CURRENT_A_PER_COUNT   0.2580f
#define PHASE_V_PER_COUNT     0.0489f
#define DC_V_PER_COUNT        0.0404f

#define VIN12_R_TOP_OHM       316000.0f
#define VIN12_R_BOTTOM_OHM    6650.0f
#define KTY_PULLUP_OHM        1000.0f

volatile uint32_t g_adc_dma[BENCH_ADC_CHANNELS];
bench_measurements_t g_measurements;

typedef struct { float temp_c; float resistance_ohm; } kty_point_t;

static const kty_point_t g_kty_table[] = {
    {-55.0f,490.0f},{-50.0f,515.0f},{-40.0f,567.0f},{-30.0f,624.0f},
    {-20.0f,684.0f},{-10.0f,747.0f},{0.0f,815.0f},{10.0f,886.0f},
    {20.0f,961.0f},{25.0f,1000.0f},{30.0f,1040.0f},{40.0f,1122.0f},
    {50.0f,1209.0f},{60.0f,1299.0f},{70.0f,1392.0f},{80.0f,1490.0f},
    {90.0f,1591.0f},{100.0f,1696.0f},{110.0f,1805.0f},{120.0f,1915.0f},
    {125.0f,1970.0f},{130.0f,2023.0f},{140.0f,2124.0f},{150.0f,2211.0f}
};

static const uint8_t g_adc_channels[BENCH_ADC_CHANNELS] = {
    ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3,
    ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_8,
    ADC_CHANNEL_9, ADC_CHANNEL_10, ADC_CHANNEL_11, ADC_CHANNEL_12,
    ADC_CHANNEL_13, ADC_CHANNEL_14, ADC_CHANNEL_15
};

static float adc_to_volts(uint16_t raw)
{
    return ((float)raw * ADC_VREF_V) / ADC_FULL_SCALE;
}

static float kty_resistance_to_temp(float r)
{
    const size_t n = sizeof(g_kty_table) / sizeof(g_kty_table[0]);
    if (r <= g_kty_table[0].resistance_ohm) return g_kty_table[0].temp_c;
    if (r >= g_kty_table[n-1U].resistance_ohm) return g_kty_table[n-1U].temp_c;

    for (size_t i=1U; i<n; ++i) {
        if (r <= g_kty_table[i].resistance_ohm) {
            const float r0=g_kty_table[i-1U].resistance_ohm;
            const float r1=g_kty_table[i].resistance_ohm;
            const float t0=g_kty_table[i-1U].temp_c;
            const float t1=g_kty_table[i].temp_c;
            return t0 + (r-r0)*(t1-t0)/(r1-r0);
        }
    }
    return 0.0f;
}

static float kty_adc_to_temp(uint16_t raw)
{
    const float v=adc_to_volts(raw);
    if (v<=0.001f) return -55.0f;
    if (v>=ADC_VREF_V-0.001f) return 150.0f;
    return kty_resistance_to_temp(KTY_PULLUP_OHM*v/(ADC_VREF_V-v));
}

static void analog_gpio_init(void)
{
    gpio_init_type gpio;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK,TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK,TRUE);
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK,TRUE);

    gpio_default_para_init(&gpio);
    gpio.gpio_mode=GPIO_MODE_ANALOG;
    gpio.gpio_pull=GPIO_PULL_NONE;

    gpio.gpio_pins=GPIO_PINS_0|GPIO_PINS_1|GPIO_PINS_2|GPIO_PINS_3|
                   GPIO_PINS_4|GPIO_PINS_5|GPIO_PINS_6;
    gpio_init(GPIOA,&gpio);

    gpio.gpio_pins=GPIO_PINS_0|GPIO_PINS_1;
    gpio_init(GPIOB,&gpio);

    gpio.gpio_pins=GPIO_PINS_0|GPIO_PINS_1|GPIO_PINS_2|
                   GPIO_PINS_3|GPIO_PINS_4|GPIO_PINS_5;
    gpio_init(GPIOC,&gpio);

    /* PB12/PB13 enable the two external temperature pull-up circuits. */
    gpio_default_para_init(&gpio);
    gpio.gpio_mode=GPIO_MODE_OUTPUT;
    gpio.gpio_out_type=GPIO_OUTPUT_PUSH_PULL;
    gpio.gpio_drive_strength=GPIO_DRIVE_STRENGTH_STRONGER;
    gpio.gpio_pull=GPIO_PULL_NONE;
    gpio.gpio_pins=GPIO_PINS_12|GPIO_PINS_13;
    gpio_init(GPIOB,&gpio);
    gpio_bits_reset(GPIOB,GPIO_PINS_12|GPIO_PINS_13);
}

static void adc_dma_init(void)
{
    dma_init_type dma;
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK,TRUE);
    dma_reset(DMA1_CHANNEL1);
    dma_default_para_init(&dma);
    dma.buffer_size=BENCH_ADC_CHANNELS;
    dma.direction=DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma.memory_base_addr=(uint32_t)g_adc_dma;
    dma.memory_data_width=DMA_MEMORY_DATA_WIDTH_WORD;
    dma.memory_inc_enable=TRUE;
    dma.peripheral_base_addr=(uint32_t)&ADC1->odt;
    dma.peripheral_data_width=DMA_PERIPHERAL_DATA_WIDTH_WORD;
    dma.peripheral_inc_enable=FALSE;
    dma.priority=DMA_PRIORITY_HIGH;
    dma.loop_mode_enable=TRUE;
    dma_init(DMA1_CHANNEL1,&dma);
    dma_channel_enable(DMA1_CHANNEL1,TRUE);
}

void bench_adc_init(void)
{
    adc_base_config_type adc;
    analog_gpio_init();
    adc_dma_init();

    crm_periph_clock_enable(CRM_ADC1_PERIPH_CLOCK,TRUE);
    crm_adc_clock_div_set(CRM_ADC_DIV_4);
    adc_combine_mode_select(ADC_INDEPENDENT_MODE);

    adc_base_default_para_init(&adc);
    adc.sequence_mode=TRUE;
    adc.repeat_mode=TRUE;
    adc.data_align=ADC_RIGHT_ALIGNMENT;
    adc.ordinary_channel_length=BENCH_ADC_CHANNELS;
    adc_base_config(ADC1,&adc);

    for (uint8_t i=0U;i<BENCH_ADC_CHANNELS;++i) {
        adc_ordinary_channel_set(ADC1,g_adc_channels[i],(uint8_t)(i+1U),ADC_SAMPLETIME_239_5);
    }

    adc_dma_mode_enable(ADC1,TRUE);
    adc_ordinary_conversion_trigger_set(ADC1,ADC12_ORDINARY_TRIG_SOFTWARE,TRUE);
    adc_enable(ADC1,TRUE);

    for (volatile uint32_t i=0U;i<100000U;++i) { __NOP(); }

    adc_calibration_init(ADC1);
    while(adc_calibration_init_status_get(ADC1)) {}
    adc_calibration_start(ADC1);
    while(adc_calibration_status_get(ADC1)) {}

    adc_ordinary_software_trigger_enable(ADC1,TRUE);
}

/* RPM: PB5 -> TMR3_CH2, 1 MHz free-running capture counter. */
static volatile uint32_t g_tmr3_overflows=0U;
static volatile uint32_t g_last_capture_ext=0U;
static volatile uint32_t g_last_pulse_ms=0U;
volatile float g_rpm=0.0f;
static volatile uint8_t g_have_capture=0U;

void bench_rpm_init(void)
{
    gpio_init_type gpio;
    tmr_input_config_type input;
    crm_clocks_freq_type clocks;

    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK,TRUE);
    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK,TRUE);
    crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK,TRUE);
    gpio_pin_remap_config(TMR3_GMUX_0010,TRUE);

    gpio_default_para_init(&gpio);
    gpio.gpio_mode=GPIO_MODE_INPUT;
    gpio.gpio_pull=GPIO_PULL_DOWN;
    gpio.gpio_pins=GPIO_PINS_5;
    gpio_init(GPIOB,&gpio);

    crm_clocks_freq_get(&clocks);
    {
        const uint32_t timer_clock_hz=clocks.apb1_freq*2U;
        const uint16_t prescaler=(uint16_t)((timer_clock_hz/1000000U)-1U);
        tmr_base_init(TMR3,0xFFFFU,prescaler);
    }
    tmr_cnt_dir_set(TMR3,TMR_COUNT_UP);

    tmr_input_default_para_init(&input);
    input.input_channel_select=TMR_SELECT_CHANNEL_2;
    input.input_polarity_select=TMR_INPUT_RISING_EDGE;
    input.input_mapped_select=TMR_CC_CHANNEL_MAPPED_DIRECT;
    input.input_filter_value=4U;
    tmr_input_channel_init(TMR3,&input,TMR_CHANNEL_INPUT_DIV_1);

    tmr_flag_clear(TMR3,TMR_OVF_FLAG);
    tmr_flag_clear(TMR3,TMR_C2_FLAG);
    tmr_interrupt_enable(TMR3,TMR_OVF_INT,TRUE);
    tmr_interrupt_enable(TMR3,TMR_C2_INT,TRUE);
    nvic_irq_enable(TMR3_GLOBAL_IRQn,1U,0U);
    tmr_counter_enable(TMR3,TRUE);
}

void TMR3_GLOBAL_IRQHandler(void)
{
    if(tmr_flag_get(TMR3,TMR_C2_FLAG)!=RESET) {
        const uint16_t capture=(uint16_t)tmr_channel_value_get(TMR3,TMR_SELECT_CHANNEL_2);
        uint32_t ovf=g_tmr3_overflows;
        if((tmr_flag_get(TMR3,TMR_OVF_FLAG)!=RESET)&&(capture<0x8000U)) ++ovf;
        const uint32_t extended=(ovf<<16)|(uint32_t)capture;

        if(g_have_capture) {
            const uint32_t delta_us=extended-g_last_capture_ext;
            if(delta_us>0U)
                g_rpm=(60.0f*1000000.0f)/(BENCH_RPM_PULSES_PER_REV*(float)delta_us);
        } else {
            g_have_capture=1U;
        }
        g_last_capture_ext=extended;
        g_last_pulse_ms=timebase_millis();
        tmr_flag_clear(TMR3,TMR_C2_FLAG);
    }

    if(tmr_flag_get(TMR3,TMR_OVF_FLAG)!=RESET) {
        ++g_tmr3_overflows;
        tmr_flag_clear(TMR3,TMR_OVF_FLAG);
    }
}

float bench_rpm_get(void)
{
    if(!g_have_capture) return 0.0f;
    if((uint32_t)(timebase_millis()-g_last_pulse_ms)>500U) return 0.0f;
    return g_rpm;
}

void bench_measurements_read(bench_measurements_t *out)
{
    if(out==NULL) return;
    for(uint8_t i=0U;i<BENCH_ADC_CHANNELS;++i) out->raw[i]=(uint16_t)g_adc_dma[i];

    out->torque_nm=((float)out->raw[ADCI_TORQUE]-ADC_CENTER_COUNTS)*TORQUE_NM_PER_COUNT;

    out->ia_a=((float)out->raw[ADCI_IA]-ADC_CENTER_COUNTS)*CURRENT_A_PER_COUNT;
    out->ib_a=((float)out->raw[ADCI_IB]-ADC_CENTER_COUNTS)*CURRENT_A_PER_COUNT;
    out->ic_a=((float)out->raw[ADCI_IC]-ADC_CENTER_COUNTS)*CURRENT_A_PER_COUNT;
    out->idc1_a=((float)out->raw[ADCI_IDC1]-ADC_CENTER_COUNTS)*CURRENT_A_PER_COUNT;
    out->idc2_a=((float)out->raw[ADCI_IDC2]-ADC_CENTER_COUNTS)*CURRENT_A_PER_COUNT;

    out->ua_v=((float)out->raw[ADCI_UA]-ADC_CENTER_COUNTS)*PHASE_V_PER_COUNT;
    out->ub_v=((float)out->raw[ADCI_UB]-ADC_CENTER_COUNTS)*PHASE_V_PER_COUNT;
    out->uc_v=((float)out->raw[ADCI_UC]-ADC_CENTER_COUNTS)*PHASE_V_PER_COUNT;
    out->udc1_v=(float)out->raw[ADCI_UDC1]*DC_V_PER_COUNT;
    out->udc2_v=(float)out->raw[ADCI_UDC2]*DC_V_PER_COUNT;

    {
        const float v=adc_to_volts(out->raw[ADCI_MEAS_12V]);
        out->vin12_v=v*(VIN12_R_TOP_OHM+VIN12_R_BOTTOM_OHM)/VIN12_R_BOTTOM_OHM;
    }

    out->t_board_c=kty_adc_to_temp(out->raw[ADCI_T_BOARD]);
    out->t_ext1_v=adc_to_volts(out->raw[ADCI_T_EXT1]);
    out->t_ext2_v=adc_to_volts(out->raw[ADCI_T_EXT2]);
    out->rpm=bench_rpm_get();
}
