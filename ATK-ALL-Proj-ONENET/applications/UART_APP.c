#include "UART_APP.h"
#include "FileSystem.h"

#define SAMPLE_UART_NAME       "uart2"      /* 串口设备名称 */

#define MB_LEN (4)
#define MP_LEN (4)
#define MP_BLOCK_SIZE  RT_ALIGN(sizeof(struct RS485_msg),sizeof(intptr_t))
//温湿度采集事件标志
#define EVENT_TH_FLAG 		(1 << 3)

#define HIGHT_TEMPVALUE 	(35.0f)
#define LOW_TEMPVALUE 		(15.0f)
#define HIGHT_HUMIVALUE 	(85.0f)
#define LOW_HUMIVALUE			(25.0f)
#define MAX_COUNTER 			(8)

//struct RS485_msg
//{
//		float light;
//    float temp;
//    float hum;
//};

unsigned char sensor_T_H[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B}; //温湿度传感器
int16_t temp = 0;   //定义温度变量
int16_t humi = 0;   //定义湿度变量
extern float light_value;
//
/* 串口接收消息结构*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

//串口设备句柄
rt_device_t serial;
//消息队列控制块
static struct rt_messagequeue rx_mq;
//
rt_mailbox_t Sensor_msg_mb;		//消息邮箱--传感器数据
rt_mp_t Sensor_msg_mp;				//存放数据内存池
rt_event_t Sensor_event;			//传感器采集事件

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));

    if ( result == -RT_EFULL)
    {
        /* 消息队列满 */
        rt_kprintf("message queue full！\n");
    }

    return result;
}

static void serial_thread_entry(void *parameter)
{
    static struct rx_msg msg;
    static rt_err_t result;
    static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

    //传感器数据结构体
    struct RS485_msg *sensor_msg;

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));

        rt_device_write(serial, 0, sensor_T_H, sizeof(sensor_T_H));  //发送采集数据指令

        /* 从消息队列中读取消息*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);

        if (result == RT_EOK)
        {
            /* 从串口读取数据*/
            rt_device_read(msg.dev, 0, rx_buffer, msg.size);

            //申请一块内存，内存满了挂起等待
            sensor_msg = rt_mp_alloc(Sensor_msg_mp, RT_WAITING_FOREVER);
            sensor_msg->temp = 1.0f * ((rx_buffer[3] << 8) + rx_buffer[4]) / 100;
            sensor_msg->hum = 1.0f * ((rx_buffer[5] << 8) + rx_buffer[6]) / 100;
            sensor_msg->light = light_value;

            temp = (int16_t)sensor_msg->temp;
            humi = (int16_t)sensor_msg->hum;

            //阈值判断逻辑
            Save_Data_TOSD((float)sensor_msg->temp, (float)sensor_msg->hum);

            //发送内存块到消息邮箱
            rt_mb_send(Sensor_msg_mb, (rt_ubase_t)sensor_msg);

            sensor_msg = NULL;

            rt_thread_mdelay(2000);
        }
    }
}

void Save_Data_TOSD(float data1, float data2)
{
    static Detect_Logic detect_logic;

    if(data1 >= HIGHT_TEMPVALUE)
    {
        detect_logic.T_Count_Alarm++;
    }
    else
    {
        detect_logic.T_Count_Alarm--;

        if(detect_logic.T_Count_Alarm <= 0)
            detect_logic.T_Count_Alarm = 0;
    }

    if(detect_logic.T_Count_Alarm >= MAX_COUNTER)
    {
        detect_logic.T_Count_Alarm = MAX_COUNTER;
        rt_kprintf("温度超过标准！%d\n", detect_logic.T_Count_Alarm);
        memset((char*)detect_logic.Alarm_buff, 0x00, sizeof(detect_logic.Alarm_buff));
        sprintf((char*)detect_logic.Alarm_buff, "Temp:%.2f,humi:%.2f\r\n", data1, data2); //拼接到温度数组里
        //保存异常数据到SD卡
        Sensor_DataTo_SD((char*)detect_logic.Alarm_buff);
    }

    //
    if(data2 >= HIGHT_HUMIVALUE)
    {
        detect_logic.H_Count_Alarm++;
    }
    else
    {
        detect_logic.H_Count_Alarm--;

        if(detect_logic.H_Count_Alarm <= 0)
            detect_logic.H_Count_Alarm = 0;
    }

    if(detect_logic.H_Count_Alarm >= MAX_COUNTER)
    {
        detect_logic.H_Count_Alarm = MAX_COUNTER;
        rt_kprintf("湿度超过标准！%d\n", detect_logic.H_Count_Alarm);
        memset((char*)detect_logic.Alarm_buff, 0x00, sizeof(detect_logic.Alarm_buff));
        sprintf((char*)detect_logic.Alarm_buff, "Temp:%.2f,humi:%.2f\r\n", data1, data2); //拼接到温度数组里
        //保存异常数据到SD卡
        Sensor_DataTo_SD((char*)detect_logic.Alarm_buff);
    }
}


static int uart_dma_thread()
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    static char msg_pool[256];

    Sensor_msg_mb = rt_mb_create("Sensor_mb", MB_LEN, RT_IPC_FLAG_FIFO);
    RT_ASSERT(Sensor_msg_mb);
    Sensor_msg_mp = rt_mp_create("Sensor_mp", MP_LEN, MP_BLOCK_SIZE);
    RT_ASSERT(Sensor_msg_mp);
    Sensor_event = rt_event_create("Sensor_event", RT_IPC_FLAG_FIFO);
    RT_ASSERT(Sensor_event);

    /* 初始化配置参数 */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    /* 查找串口设备 */
    serial = rt_device_find(SAMPLE_UART_NAME);

    config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
    config.parity    = PARITY_NONE;           //无奇偶校验位
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* 初始化消息队列 */
    rt_mq_init(&rx_mq, "rx_mq",
               msg_pool,                 /* 存放消息的缓冲区 */
               sizeof(struct rx_msg),    /* 一条消息的最大长度 */
               sizeof(msg_pool),         /* 存放消息的缓冲区大小 */
               RT_IPC_FLAG_FIFO);        /* 如果有多个线程等待，按照先来先得到的方法分配消息 */

    /* 以 DMA 接收及轮询发送方式打开串口设备 */
    rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_input);

    /* 创建 serial 线程 */
    rt_thread_t Usart2_thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 2048, 25, 20);

    if (Usart2_thread != RT_NULL)
    {
        rt_thread_startup(Usart2_thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}
/* 导出到 msh 命令列表中 */
INIT_APP_EXPORT(uart_dma_thread);
