/**
 * @file lv_test_theme_1.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_test_theme_1.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>

#include "uart_app.h"

#if LV_USE_TESTS
/*********************
 *      DEFINES
 *********************/
/* defined the LED0 pin: PB5 */
#define LED0_PIN    GET_PIN(B, 5)
/* defined the LED1 pin: PE5 */
#define LED1_PIN    GET_PIN(E, 5)
/**********************
 *      TYPEDEFS
 **********************/
#define POINT_COUNT   10  //每条数据线所具有的数据点个数

/**********************
 *  STATIC PROTOTYPES
 **********************/
//static void create_tab1(lv_obj_t * parent);
//static void create_tab2(lv_obj_t * parent);
//static void create_tab3(lv_obj_t * parent);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_style_t gauge_style;
lv_style_t chart_style;

lv_obj_t * gauge1;
lv_obj_t * chart1;

lv_obj_t * label1;
lv_obj_t * label_btn1;
lv_obj_t * label_btn2;

lv_obj_t * btn1, * btn2;

lv_chart_series_t * series1;
lv_chart_series_t * series2;

//结构体赋值
lv_color_t needle_colors1[1];//每一根指针的颜色

/**********************
 *      MACROS
 **********************/
extern float light_value;
extern int16_t temp;  //定义温度变量
extern int16_t humi;  //定义湿度变量
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
const lv_coord_t series1_y[POINT_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const lv_coord_t series2_y[POINT_COUNT] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void lv_test_theme_1(lv_theme_t * th)
{
    lv_theme_set_current(th);
    th = lv_theme_get_current();
    lv_obj_t * scr = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(scr);

    lv_obj_t * tv = lv_tabview_create(scr, NULL);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_t * tab1 = lv_tabview_add_tab(tv, "RB_Team");

    //使能滑动
    lv_tabview_set_sliding(tv, RT_FALSE);

    lv_gauge_test_start(tab1);
}

//任务回调函数
void task_cb(lv_task_t * task)
{
    char buff1[40];
    char buff2[40];

    //往series1数据线上添加新的数据点
    lv_chart_set_next(chart1, series1, temp);

    //往series2数据线上添加新的数据点
    lv_chart_set_next(chart1, series2, humi);

    //设置指针的数值
    lv_gauge_set_value(gauge1, 0, (rt_int16_t)light_value * 10);
    //然后根据不同大小的数值显示出不同的文本颜色
    alarm_leval(buff1, buff2);
}

//button回调
static void event_handler(lv_obj_t* obj, lv_event_t event)
{
    static uint8_t count1, count2;

    if (event == LV_EVENT_CLICKED)
    {
        if(obj == btn1)
        {
						count1++;
            rt_pin_write(LED0_PIN, count1 % 2);
        }

        if(obj == btn2)
        {
						count2++;
            rt_pin_write(LED1_PIN, count2 % 2);
        }
    }
}

void lv_gauge_test_start(lv_obj_t * parent)
{
    btn1 = lv_btn_create(parent, NULL);/*创建btn1*/
    lv_obj_set_event_cb(btn1, event_handler);/*设置btn1回调函数*/
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 80, -40);
    lv_obj_set_size(btn1, 100, 50);

    label_btn1 = lv_label_create(btn1, NULL);/*btn1内创建label*/
    lv_label_set_text(label_btn1, "LED1");

    btn2 = lv_btn_create(parent, NULL);/*创建btn1*/
    lv_obj_set_event_cb(btn2, event_handler);/*设置btn1回调函数*/
    lv_obj_align(btn2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 220, -40);
    lv_obj_set_size(btn2, 100, 50);

    label_btn2 = lv_label_create(btn2, NULL);/*btn1内创建label*/
    lv_label_set_text(label_btn2, "LED2");


    //1.创建自定义样式
    lv_style_copy(&gauge_style, &lv_style_pretty_color);
    gauge_style.body.main_color = LV_COLOR_MAKE(0x5F, 0xB8, 0x78); //关键数值点之前的刻度线的起始颜色,为浅绿色
    gauge_style.body.grad_color =  LV_COLOR_MAKE(0xFF, 0xB8, 0x00); //关键数值点之前的刻度线的终止颜色,为浅黄色
    gauge_style.body.padding.left = 13;//每一条刻度线的长度
    gauge_style.body.padding.inner = 8;//数值标签与刻度线之间的距离
    gauge_style.body.border.color = LV_COLOR_MAKE(0x33, 0x33, 0x33); //中心圆点的颜色
    gauge_style.line.width = 4;//刻度线的宽度
    gauge_style.text.color = LV_COLOR_WHITE;//数值标签的文本颜色
    gauge_style.line.color = LV_COLOR_OLIVE;//关键数值点之后的刻度线的颜色

    //仪表盘1
    gauge1 = lv_gauge_create(parent, NULL);//创建仪表盘
    lv_obj_set_size(gauge1, 300, 300); //设置仪表盘的大小
    lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &gauge_style); //设置样式
    lv_gauge_set_range(gauge1, 0, 50); //设置仪表盘的范围
    needle_colors1[0] = LV_COLOR_TEAL;
    lv_gauge_set_needle_count(gauge1, 1, needle_colors1); //设置指针的数量和其颜色
    lv_gauge_set_value(gauge1, 0, (rt_int16_t)light_value * 10); //设置指针1指向的数值,我们把指针1当作速度指针吧
    lv_gauge_set_critical_value(gauge1, 40); //设置关键数值点
    lv_gauge_set_scale(gauge1, 240, 41, 10); //设置角度,刻度线的数量,数值标签的数量
    lv_obj_align(gauge1, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0); //设置与屏幕居中对齐

    //3.创建一个标签来显示指针1的数值
    label1 = lv_label_create(parent, NULL);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK); //设置长文本模式
    lv_obj_set_width(label1, 80); //设置固定的宽度
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER); //设置文本居中对齐
    lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &lv_style_pretty); //设置样式
    lv_label_set_body_draw(label1, true); //使能背景重绘制
    lv_obj_align(label1, gauge1, LV_ALIGN_CENTER, 0, 60); //设置与gauge1的对齐方式
    lv_label_set_text(label1, "0.0 V/h"); //设置文本
    lv_label_set_recolor(label1, true); //使能文本重绘色

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //1.创建样式
    lv_style_copy(&chart_style, &lv_style_pretty);
    chart_style.body.main_color = LV_COLOR_WHITE;//主背景为纯白色
    chart_style.body.grad_color = chart_style.body.main_color;
    chart_style.body.border.color = LV_COLOR_BLACK;//边框的颜色
    chart_style.body.border.width = 3;//边框的宽度
    chart_style.body.border.opa = LV_OPA_COVER;
    chart_style.body.radius = 1;
    chart_style.line.color = LV_COLOR_GRAY;//分割线和刻度线的颜色
    chart_style.text.color = LV_COLOR_WHITE;//主刻度标题的颜色

    //2.创建图表对象
    chart1 = lv_chart_create(parent, NULL);
    lv_obj_set_size(chart1, 250, 200); //设置图表的大小
    lv_obj_align(chart1, NULL, LV_ALIGN_IN_RIGHT_MID, -70, -20); //设置对齐方式
    lv_chart_set_type(chart1, LV_CHART_TYPE_LINE); //设置为折线	散点:LV_CHART_TYPE_POINT
    lv_chart_set_series_opa(chart1, LV_OPA_80); //设置数据线的透明度,不设置的话,则LV_OPA_COVER是默认值
    lv_chart_set_series_width(chart1, 4); //设置数据线的宽度
    lv_chart_set_series_darking(chart1, LV_OPA_80); //设置数据线的黑阴影效果
    lv_chart_set_style(chart1, LV_CHART_STYLE_MAIN, &chart_style); //设置样式
    lv_chart_set_point_count(chart1, POINT_COUNT); //设置每条数据线所具有的数据点个数,如果不设置的话,则默认值是10
    lv_chart_set_div_line_count(chart1, 4, 4); //设置水平和垂直分割线
    lv_chart_set_range(chart1, 0, 100); //设置y轴的数值范围,[0,100]也是默认值
    lv_chart_set_y_tick_length(chart1, 10, 3); //设置y轴的主刻度线长度和次刻度线长度
    lv_chart_set_y_tick_texts(chart1, "100\n90\n80\n70\n60\n50\n40\n30\n20\n10\n0", 5, LV_CHART_AXIS_DRAW_LAST_TICK); //设置y轴的主刻度标题和每个主刻度标题间的刻度数
    lv_chart_set_x_tick_length(chart1, 10, 3); //设置x轴的主刻度线长度和次刻度线长度
    lv_chart_set_x_tick_texts(chart1, "0\n2\n4\n6\n8\n10", 5, LV_CHART_AXIS_DRAW_LAST_TICK); //设置x轴的刻度数和主刻度标题
    lv_chart_set_margin(chart1, 40); //设置刻度区域的高度
    //2.2 往图表中添加第1条数据线
    series1 = lv_chart_add_series(chart1, LV_COLOR_RED); //指定为红色
    lv_chart_set_points(chart1, series1, (lv_coord_t*)series1_y); //初始化数据点的值
    //series1->points[1] = 70;//也可以采用直接修改的方式

    //2.3 往图表中添加第2条数据线
    series2 = lv_chart_add_series(chart1, LV_COLOR_BLUE); //指定为蓝色
    lv_chart_set_points(chart1, series2, (lv_coord_t*)series2_y); //初始化数据点的值

    lv_chart_refresh(chart1);//如果是采用直接修改的方式,请最好调用一下刷新操作

    //4.创建一个任务来显示变化
    lv_task_create(task_cb, 300, LV_TASK_PRIO_MID, NULL);
}


void alarm_leval(char* buff1, char* buff2)
{

    if(light_value * 10 < 20.0f)
    {
        sprintf(buff1, "#5FB878 %.2f V#", light_value * 10); //显示绿色,代表安全
    }
    else if(light_value * 10 < 40.0f)
    {
        sprintf(buff1, "#FFB800 %.2f V#", light_value * 10); //显示黄色,代表警告
    }
    else
    {
        sprintf(buff1, "#FF0000 %.2f V#", light_value * 10); //显示红色,代表危险
    }

    lv_label_set_text(label1, buff1);

}


#endif /*LV_USE_TESTS*/
