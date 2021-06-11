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
#define POINT_COUNT   10  //ÿ�������������е����ݵ����

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

//�ṹ�帳ֵ
lv_color_t needle_colors1[1];//ÿһ��ָ�����ɫ

/**********************
 *      MACROS
 **********************/
extern float light_value;
extern int16_t temp;  //�����¶ȱ���
extern int16_t humi;  //����ʪ�ȱ���
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

    //ʹ�ܻ���
    lv_tabview_set_sliding(tv, RT_FALSE);

    lv_gauge_test_start(tab1);
}

//����ص�����
void task_cb(lv_task_t * task)
{
    char buff1[40];
    char buff2[40];

    //��series1������������µ����ݵ�
    lv_chart_set_next(chart1, series1, temp);

    //��series2������������µ����ݵ�
    lv_chart_set_next(chart1, series2, humi);

    //����ָ�����ֵ
    lv_gauge_set_value(gauge1, 0, (rt_int16_t)light_value * 10);
    //Ȼ����ݲ�ͬ��С����ֵ��ʾ����ͬ���ı���ɫ
    alarm_leval(buff1, buff2);
}

//button�ص�
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
    btn1 = lv_btn_create(parent, NULL);/*����btn1*/
    lv_obj_set_event_cb(btn1, event_handler);/*����btn1�ص�����*/
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 80, -40);
    lv_obj_set_size(btn1, 100, 50);

    label_btn1 = lv_label_create(btn1, NULL);/*btn1�ڴ���label*/
    lv_label_set_text(label_btn1, "LED1");

    btn2 = lv_btn_create(parent, NULL);/*����btn1*/
    lv_obj_set_event_cb(btn2, event_handler);/*����btn1�ص�����*/
    lv_obj_align(btn2, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 220, -40);
    lv_obj_set_size(btn2, 100, 50);

    label_btn2 = lv_label_create(btn2, NULL);/*btn1�ڴ���label*/
    lv_label_set_text(label_btn2, "LED2");


    //1.�����Զ�����ʽ
    lv_style_copy(&gauge_style, &lv_style_pretty_color);
    gauge_style.body.main_color = LV_COLOR_MAKE(0x5F, 0xB8, 0x78); //�ؼ���ֵ��֮ǰ�Ŀ̶��ߵ���ʼ��ɫ,Ϊǳ��ɫ
    gauge_style.body.grad_color =  LV_COLOR_MAKE(0xFF, 0xB8, 0x00); //�ؼ���ֵ��֮ǰ�Ŀ̶��ߵ���ֹ��ɫ,Ϊǳ��ɫ
    gauge_style.body.padding.left = 13;//ÿһ���̶��ߵĳ���
    gauge_style.body.padding.inner = 8;//��ֵ��ǩ��̶���֮��ľ���
    gauge_style.body.border.color = LV_COLOR_MAKE(0x33, 0x33, 0x33); //����Բ�����ɫ
    gauge_style.line.width = 4;//�̶��ߵĿ��
    gauge_style.text.color = LV_COLOR_WHITE;//��ֵ��ǩ���ı���ɫ
    gauge_style.line.color = LV_COLOR_OLIVE;//�ؼ���ֵ��֮��Ŀ̶��ߵ���ɫ

    //�Ǳ���1
    gauge1 = lv_gauge_create(parent, NULL);//�����Ǳ���
    lv_obj_set_size(gauge1, 300, 300); //�����Ǳ��̵Ĵ�С
    lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &gauge_style); //������ʽ
    lv_gauge_set_range(gauge1, 0, 50); //�����Ǳ��̵ķ�Χ
    needle_colors1[0] = LV_COLOR_TEAL;
    lv_gauge_set_needle_count(gauge1, 1, needle_colors1); //����ָ�������������ɫ
    lv_gauge_set_value(gauge1, 0, (rt_int16_t)light_value * 10); //����ָ��1ָ�����ֵ,���ǰ�ָ��1�����ٶ�ָ���
    lv_gauge_set_critical_value(gauge1, 40); //���ùؼ���ֵ��
    lv_gauge_set_scale(gauge1, 240, 41, 10); //���ýǶ�,�̶��ߵ�����,��ֵ��ǩ������
    lv_obj_align(gauge1, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0); //��������Ļ���ж���

    //3.����һ����ǩ����ʾָ��1����ֵ
    label1 = lv_label_create(parent, NULL);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK); //���ó��ı�ģʽ
    lv_obj_set_width(label1, 80); //���ù̶��Ŀ��
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER); //�����ı����ж���
    lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &lv_style_pretty); //������ʽ
    lv_label_set_body_draw(label1, true); //ʹ�ܱ����ػ���
    lv_obj_align(label1, gauge1, LV_ALIGN_CENTER, 0, 60); //������gauge1�Ķ��뷽ʽ
    lv_label_set_text(label1, "0.0 V/h"); //�����ı�
    lv_label_set_recolor(label1, true); //ʹ���ı��ػ�ɫ

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //1.������ʽ
    lv_style_copy(&chart_style, &lv_style_pretty);
    chart_style.body.main_color = LV_COLOR_WHITE;//������Ϊ����ɫ
    chart_style.body.grad_color = chart_style.body.main_color;
    chart_style.body.border.color = LV_COLOR_BLACK;//�߿����ɫ
    chart_style.body.border.width = 3;//�߿�Ŀ��
    chart_style.body.border.opa = LV_OPA_COVER;
    chart_style.body.radius = 1;
    chart_style.line.color = LV_COLOR_GRAY;//�ָ��ߺͿ̶��ߵ���ɫ
    chart_style.text.color = LV_COLOR_WHITE;//���̶ȱ������ɫ

    //2.����ͼ�����
    chart1 = lv_chart_create(parent, NULL);
    lv_obj_set_size(chart1, 250, 200); //����ͼ��Ĵ�С
    lv_obj_align(chart1, NULL, LV_ALIGN_IN_RIGHT_MID, -70, -20); //���ö��뷽ʽ
    lv_chart_set_type(chart1, LV_CHART_TYPE_LINE); //����Ϊ����	ɢ��:LV_CHART_TYPE_POINT
    lv_chart_set_series_opa(chart1, LV_OPA_80); //���������ߵ�͸����,�����õĻ�,��LV_OPA_COVER��Ĭ��ֵ
    lv_chart_set_series_width(chart1, 4); //���������ߵĿ��
    lv_chart_set_series_darking(chart1, LV_OPA_80); //���������ߵĺ���ӰЧ��
    lv_chart_set_style(chart1, LV_CHART_STYLE_MAIN, &chart_style); //������ʽ
    lv_chart_set_point_count(chart1, POINT_COUNT); //����ÿ�������������е����ݵ����,��������õĻ�,��Ĭ��ֵ��10
    lv_chart_set_div_line_count(chart1, 4, 4); //����ˮƽ�ʹ�ֱ�ָ���
    lv_chart_set_range(chart1, 0, 100); //����y�����ֵ��Χ,[0,100]Ҳ��Ĭ��ֵ
    lv_chart_set_y_tick_length(chart1, 10, 3); //����y������̶��߳��Ⱥʹο̶��߳���
    lv_chart_set_y_tick_texts(chart1, "100\n90\n80\n70\n60\n50\n40\n30\n20\n10\n0", 5, LV_CHART_AXIS_DRAW_LAST_TICK); //����y������̶ȱ����ÿ�����̶ȱ����Ŀ̶���
    lv_chart_set_x_tick_length(chart1, 10, 3); //����x������̶��߳��Ⱥʹο̶��߳���
    lv_chart_set_x_tick_texts(chart1, "0\n2\n4\n6\n8\n10", 5, LV_CHART_AXIS_DRAW_LAST_TICK); //����x��Ŀ̶��������̶ȱ���
    lv_chart_set_margin(chart1, 40); //���ÿ̶�����ĸ߶�
    //2.2 ��ͼ������ӵ�1��������
    series1 = lv_chart_add_series(chart1, LV_COLOR_RED); //ָ��Ϊ��ɫ
    lv_chart_set_points(chart1, series1, (lv_coord_t*)series1_y); //��ʼ�����ݵ��ֵ
    //series1->points[1] = 70;//Ҳ���Բ���ֱ���޸ĵķ�ʽ

    //2.3 ��ͼ������ӵ�2��������
    series2 = lv_chart_add_series(chart1, LV_COLOR_BLUE); //ָ��Ϊ��ɫ
    lv_chart_set_points(chart1, series2, (lv_coord_t*)series2_y); //��ʼ�����ݵ��ֵ

    lv_chart_refresh(chart1);//����ǲ���ֱ���޸ĵķ�ʽ,����õ���һ��ˢ�²���

    //4.����һ����������ʾ�仯
    lv_task_create(task_cb, 300, LV_TASK_PRIO_MID, NULL);
}


void alarm_leval(char* buff1, char* buff2)
{

    if(light_value * 10 < 20.0f)
    {
        sprintf(buff1, "#5FB878 %.2f V#", light_value * 10); //��ʾ��ɫ,����ȫ
    }
    else if(light_value * 10 < 40.0f)
    {
        sprintf(buff1, "#FFB800 %.2f V#", light_value * 10); //��ʾ��ɫ,������
    }
    else
    {
        sprintf(buff1, "#FF0000 %.2f V#", light_value * 10); //��ʾ��ɫ,����Σ��
    }

    lv_label_set_text(label1, buff1);

}


#endif /*LV_USE_TESTS*/
