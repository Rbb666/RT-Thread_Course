#include "MQTT_APP.h"

struct recvdata
{
    float light;
    float temp;
    float hum;
};

#define LED0_PIN    GET_PIN(B, 5)
#define LED1_PIN    GET_PIN(E, 5)
#define LED1_ON 		rt_pin_write(LED0_PIN, PIN_LOW)
#define LED1_OFF 		rt_pin_write(LED0_PIN, PIN_HIGH)
#define LED2_ON 		rt_pin_write(LED1_PIN, PIN_LOW)
#define LED2_OFF 		rt_pin_write(LED1_PIN, PIN_HIGH)

/* 定义一个MQTT客户端结构体 */
static MQTTClient client;
//
extern rt_mailbox_t Sensor_msg_mb;	//消息邮箱--传感器数据
extern rt_event_t Sensor_event;			//传感器采集事件
extern rt_device_t serial;					//串口2设备
extern unsigned char sensor_T_H[8]; //温湿度传感器
extern rt_bool_t connect_sta;				//wifi连接状态
//
static uint8_t led1 = 0;
static uint8_t led2 = 0;
//
/* 收到订阅主题的消息时的回调函数*/
static void mqtt_sub_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *) msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    rt_kprintf("%.*s\r\n", msg_data->message->payloadlen, (char *) msg_data->message->payload);

    cJSON *root = RT_NULL, *object = RT_NULL;
    root = cJSON_Parse((const char *) msg_data->message->payload);

    if (!root)
    {
        rt_kprintf("No memory for cJSON root!\n");
        cJSON_Delete(root);
        return;
    }

    object = cJSON_GetObjectItem(root, "led1");

    if (object->type == cJSON_Number)
        led1 = object->valueint;

    object = cJSON_GetObjectItem(root, "led2");

    if (object->type == cJSON_Number)
        led2 = object->valueint;

    led1 == RT_TRUE ? LED1_ON : LED1_OFF;
    led2 == RT_TRUE ? LED2_ON : LED2_OFF;

    if (NULL != root)
    {
        cJSON_Delete(root);
        root = NULL;
    }
}

/* 默认的订阅回调函数，如果有订阅的 Topic 没有设置回调函数，则使用该默认回调函数 */
static void mqtt_sub_default_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *) msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    rt_kprintf("Receive topic: %.*s, message data:\r\n", msg_data->topicName->lenstring.len,
               msg_data->topicName->lenstring.data);
    rt_kprintf("%.*s\r\n", msg_data->message->payloadlen, (char *) msg_data->message->payload);
}

/* 连接成功回调函数 */
static void mqtt_connect_callback(MQTTClient *c)
{
    rt_kprintf("success contact to mqtt!\r\n");
}

/* 上线回调函数 */
static void mqtt_online_callback(MQTTClient *c)
{
    rt_kprintf("mqtt is online \r\n");
    client.isconnected = 1;
}

/* 下线回调函数 */
static void mqtt_offline_callback(MQTTClient *c)
{
    rt_kprintf("mqtt is offline \r\n");
}

static void app_mqtt_thread_entry(void *parameter)
{
    /* 用以存放邮件 */
    rt_ubase_t* buf;
    /* 对MQTT客户端结构体变量进行配置 */
    client.isconnected = 0;
    client.uri = MQTT_Uri;

    /* 配置MQTT的连接参数 */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;
    memcpy(&client.condata, &condata, sizeof(condata));
    client.condata.clientID.cstring = ClientId;
    client.condata.keepAliveInterval = 30;
    client.condata.cleansession = 1;
    client.condata.username.cstring = UserName;
    client.condata.password.cstring = PassWord;

    /* 为mqtt申请内存 */
    client.buf_size = client.readbuf_size = 1024;
    client.buf = rt_calloc(1, client.buf_size);
    client.readbuf = rt_calloc(1, client.readbuf_size);

    if (!(client.buf && client.readbuf))
    {
        rt_kprintf("no memory for MQTT client buffer!\r\n");
        return;
    }

    /* 设置回调函数  */
    client.connect_callback = mqtt_connect_callback;
    client.online_callback = mqtt_online_callback;
    client.offline_callback = mqtt_offline_callback;

    /* 订阅一个主题，并设置其回调函数 */
    client.messageHandlers[0].topicFilter = rt_strdup(RT_SUB_TOPIC);
    client.messageHandlers[0].callback = mqtt_sub_callback;
    client.messageHandlers[0].qos = QOS1;

    /* 设置默认的回调函数 */
    client.defaultMessageHandler = mqtt_sub_default_callback;

    /* 启动 mqtt client */
    paho_mqtt_start(&client);

    //传感器数据结构体
    struct RS485_msg *sensor_msg;
    static rt_uint32_t e;
    static char r_buff[64];

    while(1)
    {
        //ADC数据接收事件
        if (rt_event_recv(Sensor_event, EVENT_ADC_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e) != RT_EOK)
            continue;

        //5s等待接收邮件
        if(rt_mb_recv(Sensor_msg_mb, (rt_ubase_t*)&sensor_msg, rt_tick_from_millisecond(5000)) == RT_EOK)
        {
            sprintf((char*)r_buff, "{\"temperature\":%.2f,\"humidity\":%.2f,\"light\":%.2f}",
                    (float)sensor_msg->temp, (float)sensor_msg->hum, (float)sensor_msg->light); //拼接到温度数组里

            if(connect_sta == RT_TRUE && client.isconnected)
            {
                if(paho_mqtt_publish(&client, QOS1, RT_PUB_TOPIC, (char *) r_buff) != RT_EOK)
                    rt_kprintf("mqtt publish failed...\n");
                else
                    rt_kprintf("onenet upload OK >>> %s\n", r_buff);
            }

            //释放内存块
            rt_mp_free(sensor_msg);
            sensor_msg = RT_NULL;
            //继续接收
            continue;
        }

        //5s超时直接存数据
        rt_kputs("@5s接收事件超时--存储数据\n");
        rt_device_write(serial, 0, sensor_T_H, sizeof(sensor_T_H));  //发送采集数据指令
        continue;
    }
}

//mqtt_thread
static int app_mqtt_init(void)
{
    rt_err_t rt_err;

    /* 创建MQTT线程*/
    rt_thread_t app_mqtt_thread = rt_thread_create("app_mqtt", app_mqtt_thread_entry,
                                  RT_NULL, 2048, 18, 20);

    /* 如果获得线程控制块，启动这个线程 */
    if (app_mqtt_thread != RT_NULL)
        rt_err = rt_thread_startup(app_mqtt_thread);
    else
        rt_kprintf("app_mqtt_thread create failure !!! \n");

    /* 判断线程是否启动成功 */
    if (rt_err == RT_EOK)
        rt_kprintf("mqtt_thread is ok. \n");
    else
        rt_kprintf("app_mqtt_thread startup err. \n");

    return rt_err;
}

INIT_APP_EXPORT(app_mqtt_init);
