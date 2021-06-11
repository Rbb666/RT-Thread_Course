#ifndef UART_APP_H__
#define UART_APP_H__

#include <rtthread.h>
#include "drv_usart.h"
#include <stdio.h>

struct RS485_msg
{
    float light;
    float temp;
    float hum;
};
typedef struct RS485_msg *Sensor_msg;

typedef struct
{
    short T_Count_Alarm ;		/*统计报警次数*/
    short H_Count_Alarm ;		/*统计报警次数*/
    char Alarm_buff[35];
} Detect_Logic;
extern Detect_Logic detect_logic ;

float GET_485(unsigned char * hex, char* rx_buffer, int msg_bit, int wait);
void Save_Data_TOSD(float data1, float data2);

#endif
