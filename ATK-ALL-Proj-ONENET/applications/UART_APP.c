#include "UART_APP.h"
#include "FileSystem.h"

#define SAMPLE_UART_NAME       "uart2"      /* �����豸���� */

#define MB_LEN (4)
#define MP_LEN (4)
#define MP_BLOCK_SIZE  RT_ALIGN(sizeof(struct RS485_msg),sizeof(intptr_t))
//��ʪ�Ȳɼ��¼���־
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

unsigned char sensor_T_H[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B}; //��ʪ�ȴ�����
int16_t temp = 0;   //�����¶ȱ���
int16_t humi = 0;   //����ʪ�ȱ���
extern float light_value;
//
/* ���ڽ�����Ϣ�ṹ*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

//�����豸���
rt_device_t serial;
//��Ϣ���п��ƿ�
static struct rt_messagequeue rx_mq;
//
rt_mailbox_t Sensor_msg_mb;		//��Ϣ����--����������
rt_mp_t Sensor_msg_mp;				//��������ڴ��
rt_event_t Sensor_event;			//�������ɼ��¼�

/* �������ݻص����� */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));

    if ( result == -RT_EFULL)
    {
        /* ��Ϣ������ */
        rt_kprintf("message queue full��\n");
    }

    return result;
}

static void serial_thread_entry(void *parameter)
{
    static struct rx_msg msg;
    static rt_err_t result;
    static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

    //���������ݽṹ��
    struct RS485_msg *sensor_msg;

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));

        rt_device_write(serial, 0, sensor_T_H, sizeof(sensor_T_H));  //���Ͳɼ�����ָ��

        /* ����Ϣ�����ж�ȡ��Ϣ*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);

        if (result == RT_EOK)
        {
            /* �Ӵ��ڶ�ȡ����*/
            rt_device_read(msg.dev, 0, rx_buffer, msg.size);

            //����һ���ڴ棬�ڴ����˹���ȴ�
            sensor_msg = rt_mp_alloc(Sensor_msg_mp, RT_WAITING_FOREVER);
            sensor_msg->temp = 1.0f * ((rx_buffer[3] << 8) + rx_buffer[4]) / 100;
            sensor_msg->hum = 1.0f * ((rx_buffer[5] << 8) + rx_buffer[6]) / 100;
            sensor_msg->light = light_value;

            temp = (int16_t)sensor_msg->temp;
            humi = (int16_t)sensor_msg->hum;

            //��ֵ�ж��߼�
            Save_Data_TOSD((float)sensor_msg->temp, (float)sensor_msg->hum);

            //�����ڴ�鵽��Ϣ����
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
        rt_kprintf("�¶ȳ�����׼��%d\n", detect_logic.T_Count_Alarm);
        memset((char*)detect_logic.Alarm_buff, 0x00, sizeof(detect_logic.Alarm_buff));
        sprintf((char*)detect_logic.Alarm_buff, "Temp:%.2f,humi:%.2f\r\n", data1, data2); //ƴ�ӵ��¶�������
        //�����쳣���ݵ�SD��
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
        rt_kprintf("ʪ�ȳ�����׼��%d\n", detect_logic.H_Count_Alarm);
        memset((char*)detect_logic.Alarm_buff, 0x00, sizeof(detect_logic.Alarm_buff));
        sprintf((char*)detect_logic.Alarm_buff, "Temp:%.2f,humi:%.2f\r\n", data1, data2); //ƴ�ӵ��¶�������
        //�����쳣���ݵ�SD��
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

    /* ��ʼ�����ò��� */
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    /* ���Ҵ����豸 */
    serial = rt_device_find(SAMPLE_UART_NAME);

    config.baud_rate = BAUD_RATE_9600;        //�޸Ĳ�����Ϊ 9600
    config.data_bits = DATA_BITS_8;           //����λ 8
    config.stop_bits = STOP_BITS_1;           //ֹͣλ 1
    config.bufsz     = 128;                   //�޸Ļ����� buff size Ϊ 128
    config.parity    = PARITY_NONE;           //����żУ��λ
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* ��ʼ����Ϣ���� */
    rt_mq_init(&rx_mq, "rx_mq",
               msg_pool,                 /* �����Ϣ�Ļ����� */
               sizeof(struct rx_msg),    /* һ����Ϣ����󳤶� */
               sizeof(msg_pool),         /* �����Ϣ�Ļ�������С */
               RT_IPC_FLAG_FIFO);        /* ����ж���̵߳ȴ������������ȵõ��ķ���������Ϣ */

    /* �� DMA ���ռ���ѯ���ͷ�ʽ�򿪴����豸 */
    rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_input);

    /* ���� serial �߳� */
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
/* ������ msh �����б��� */
INIT_APP_EXPORT(uart_dma_thread);
