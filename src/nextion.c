#include "nextion.h"
#include "at32a403a_conf.h"
#include <stdio.h>

static void uart_send_byte(uint8_t b)
{
    while(usart_flag_get(USART3,USART_TDBE_FLAG)==RESET) {}
    usart_data_transmit(USART3,b);
}

static void uart_send(const char *s)
{
    while(*s!='\0') uart_send_byte((uint8_t)*s++);
}

static void nextion_end(void)
{
    uart_send_byte(0xFFU); uart_send_byte(0xFFU); uart_send_byte(0xFFU);
}

void nextion_init(uint32_t baudrate)
{
    gpio_init_type gpio;
    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK,TRUE);
    crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK,TRUE);
    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK,TRUE);
    gpio_pin_remap_config(USART3_GMUX_0011,TRUE);

    gpio_default_para_init(&gpio);
    gpio.gpio_drive_strength=GPIO_DRIVE_STRENGTH_STRONGER;
    gpio.gpio_out_type=GPIO_OUTPUT_PUSH_PULL;
    gpio.gpio_mode=GPIO_MODE_MUX;
    gpio.gpio_pull=GPIO_PULL_NONE;
    gpio.gpio_pins=GPIO_PINS_8;
    gpio_init(GPIOD,&gpio);

    gpio_default_para_init(&gpio);
    gpio.gpio_mode=GPIO_MODE_INPUT;
    gpio.gpio_pull=GPIO_PULL_UP;
    gpio.gpio_pins=GPIO_PINS_9;
    gpio_init(GPIOD,&gpio);

    usart_init(USART3,baudrate,USART_DATA_8BITS,USART_STOP_1_BIT);
    usart_parity_selection_config(USART3,USART_PARITY_NONE);
    usart_transmitter_enable(USART3,TRUE);
    usart_receiver_enable(USART3,TRUE);
    usart_enable(USART3,TRUE);
}

void nextion_command(const char *command)
{
    if(command==NULL) return;
    uart_send(command);
    nextion_end();
}

void nextion_set_text(const char *object,const char *text)
{
    char cmd[96];
    if((object==NULL)||(text==NULL)) return;
    (void)snprintf(cmd,sizeof(cmd),"%s.txt=\"%s\"",object,text);
    nextion_command(cmd);
}

void nextion_set_float1(const char *object,float value)
{
    char cmd[64];
    int32_t scaled=(int32_t)(value*10.0f+((value>=0.0f)?0.5f:-0.5f));
    const int negative=(scaled<0);
    if(negative) scaled=-scaled;
    const int32_t whole=scaled/10;
    const int32_t frac=scaled%10;
    (void)snprintf(cmd,sizeof(cmd),negative?"%s.txt=\"-%ld.%ld\"":"%s.txt=\"%ld.%ld\"",
                   object,(long)whole,(long)frac);
    nextion_command(cmd);
}

void nextion_set_u32(const char *object,uint32_t value)
{
    char cmd[64];
    (void)snprintf(cmd,sizeof(cmd),"%s.txt=\"%lu\"",object,(unsigned long)value);
    nextion_command(cmd);
}

void nextion_show_measurements(const bench_measurements_t *m)
{
    if(m==NULL) return;
    nextion_set_float1("tRpm",m->rpm);
    nextion_set_float1("tTorque",m->torque_nm);
    nextion_set_float1("tIa",m->ia_a);
    nextion_set_float1("tIb",m->ib_a);
    nextion_set_float1("tIc",m->ic_a);
    nextion_set_float1("tIdc1",m->idc1_a);
    nextion_set_float1("tIdc2",m->idc2_a);
    nextion_set_float1("tUa",m->ua_v);
    nextion_set_float1("tUb",m->ub_v);
    nextion_set_float1("tUc",m->uc_v);
    nextion_set_float1("tUdc1",m->udc1_v);
    nextion_set_float1("tUdc2",m->udc2_v);
    nextion_set_float1("tVin12",m->vin12_v);
    nextion_set_float1("tTbrd",m->t_board_c);
    nextion_set_float1("tText1",m->t_ext1_v);
    nextion_set_float1("tText2",m->t_ext2_v);
    nextion_set_u32("tRawTrq",m->raw[ADCI_TORQUE]);
}
