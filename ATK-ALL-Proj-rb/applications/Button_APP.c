#include "button_app.h"
#include "FileSystem.h"

extern struct key_state_type key0;
extern struct key_state_type key1;

static Button_t button_0;
static Button_t button_1;

//����key ���ƿ�
struct key_state_type key0 = { 0 };
struct key_state_type key1 = { 0 };

//read button 0 level
uint8_t button0_read_level(void)
{
    return rt_pin_read(KEY_0);
}

//button 0 down callback
void button0_down_callback(void *btn)
{
    key0.down_state = KEY_PRESS;
}

// button 0 long callback
void button0_double_callback(void *btn)
{
    key0.double_state = KEY_PRESS;
}

// button 0 long callback
void button0_long_callback(void *btn)
{
    key0.long_state = KEY_PRESS;
}

//read button 1 level
uint8_t button1_read_level(void)
{
    return rt_pin_read(KEY_1);
}
//button1 down callback
void button1_down_callback(void *btn)
{
    key1.down_state = KEY_PRESS;
}
// button1 long callback
void button1_double_callback(void *btn)
{
    key1.double_state = KEY_PRESS;
}
// button 1 long callback
void button1_long_callback(void *btn)
{
    key1.long_state = KEY_PRESS;
}

void key_process(void)
{
    Button_Process();
}

int	key_init(void)
{
    //button gpio init
    rt_pin_mode(KEY_0, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_1, PIN_MODE_INPUT_PULLUP);

    //button create
    Button_Create("button_0", &button_0, button0_read_level, PIN_LOW);
    Button_Create("button_1", &button_1, button1_read_level, PIN_LOW);

    //set button callback
    Button_Attach(&button_0, BUTTON_DOWM, button0_down_callback);
    Button_Attach(&button_0, BUTTON_DOUBLE, button0_double_callback);
    Button_Attach(&button_0, BUTTON_LONG, button0_long_callback);

    Button_Attach(&button_1, BUTTON_DOWM, button1_down_callback);
    Button_Attach(&button_1, BUTTON_DOUBLE, button1_double_callback);
    Button_Attach(&button_1, BUTTON_LONG, button1_long_callback);

    return RT_EOK;
}

INIT_APP_EXPORT(key_init);

void KEY_APP_Task(void)
{
    key_process();

    if(key0.double_state == KEY_PRESS)
    {
        rt_kprintf("key0 is double_pass\n");

        Sensor_DataTo_SD("test");

        key0.double_state = KEY_RELEASE;
    }
    else if (key0.long_state == KEY_PRESS)
    {
        rt_kprintf("key0 is long_pass\n");

        Data_ReadFSD();

        key0.long_state = KEY_RELEASE;
    }
}
