#ifndef BENCH_MONITOR_H
#define BENCH_MONITOR_H
#include <stdint.h>

#define BENCH_ADC_CHANNELS 15U
#define BENCH_RPM_PULSES_PER_REV 60.0f

typedef enum {
    ADCI_TORQUE = 0, ADCI_IA, ADCI_IB, ADCI_IC, ADCI_IDC1, ADCI_IDC2,
    ADCI_T_BOARD, ADCI_T_EXT1, ADCI_T_EXT2, ADCI_MEAS_12V,
    ADCI_UA, ADCI_UB, ADCI_UC, ADCI_UDC1, ADCI_UDC2
} bench_adc_index_t;

typedef struct {
    uint16_t raw[BENCH_ADC_CHANNELS];
    float torque_nm;
    float ia_a, ib_a, ic_a, idc1_a, idc2_a;
    float ua_v, ub_v, uc_v, udc1_v, udc2_v, vin12_v;
    float t_board_c;
    float t_ext1_v, t_ext2_v;
    float rpm;
} bench_measurements_t;

void bench_adc_init(void);
void bench_rpm_init(void);
void bench_measurements_read(bench_measurements_t *out);
float bench_rpm_get(void);
#endif
