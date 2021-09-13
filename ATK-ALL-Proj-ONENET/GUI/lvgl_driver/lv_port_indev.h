#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

enum littlevgl2rtt_input_state
{
    LITTLEVGL2RTT_INPUT_NONE = 0x00,
    LITTLEVGL2RTT_INPUT_UP   = 0x01, 
    LITTLEVGL2RTT_INPUT_DOWN = 0x02, 
    LITTLEVGL2RTT_INPUT_MOVE = 0x03
};

//º¯ÊýÉêÃ÷
void lv_port_indev_init(void);

#ifdef __cplusplus
} 
#endif

#endif

