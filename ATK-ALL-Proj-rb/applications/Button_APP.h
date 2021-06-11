#ifndef BUTTPN_APP_H__
#define BUTTPN_APP_H__

#include "button.h"
#include <board.h>

#define KEY_0		GET_PIN(E, 4)
#define	KEY_1		GET_PIN(E, 3)

#define KEY_PRESS  	PIN_HIGH
#define KEY_RELEASE PIN_LOW

//定义key 状态结构体
struct key_state_type
{
    int down_state;
    int double_state;
    int long_state;
};

void key_process(void);
void KEY_APP_Task(void);

#endif

