#ifndef NEXTION_H
#define NEXTION_H
#include <stdint.h>
#include "bench_monitor.h"
void nextion_init(uint32_t baudrate);
void nextion_command(const char *command);
void nextion_set_text(const char *object, const char *text);
void nextion_set_float1(const char *object, float value);
void nextion_set_u32(const char *object, uint32_t value);
void nextion_show_measurements(const bench_measurements_t *m);
#endif
