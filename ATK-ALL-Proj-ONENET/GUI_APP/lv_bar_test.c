#include "lv_bar_test.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_tests\lv_test_theme\lv_test_theme_1.h"
#include "lv_tests\lv_test_theme\lv_test_theme_2.h"

#include "touch\touch.h"
#include "button_app.h"


static int _lv_init = 0;
static void lvgl_tick_run(void *p)
{
    if (_lv_init)
    {
        lv_tick_inc(1);
    }
}

static int lvgl_tick_handler_init(void)
{
    rt_timer_t timer = RT_NULL;
    int ret;

    timer = rt_timer_create("lv_tick", lvgl_tick_run, RT_NULL, 1, RT_TIMER_FLAG_PERIODIC);

    if (timer == RT_NULL)
    {
        return RT_ERROR;
    }

    ret = rt_timer_start(timer);
    return ret;
}

static void lvgl_th_run(void *p)
{
    tp_dev.init();				//触摸屏初始化
    //
    lv_init();					//lvgl系统初始化
    lv_port_disp_init();		//lvgl显示接口初始化
    lv_port_indev_init();		//lvgl输入接口初始化
    _lv_init = 1;				//开启心跳
    lvgl_tick_handler_init();	//心跳定时器
    //
    lv_test_theme_1(lv_theme_night_init(210, NULL));

    while(1)
    {
        tp_dev.scan(0);
        lv_task_handler();
        KEY_APP_Task();
        rt_thread_mdelay(5);
    }
}

int rt_lvgl_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_thread_t thread = RT_NULL;

    thread = rt_thread_create("lvgl", lvgl_th_run, RT_NULL, 2048, 15, 20);

    if(thread == RT_NULL)
    {
        return RT_ERROR;
    }

    rt_thread_startup(thread);

    return RT_EOK;
}
INIT_APP_EXPORT(rt_lvgl_init);
