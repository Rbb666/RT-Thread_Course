/*
 * �����嵥������һ�� ���� �豸ʹ������
 * ���̵����� uart_sample ��������ն�
 * ������ø�ʽ��uart_sample uart2
 * ������ͣ�����ڶ���������Ҫʹ�õĴ����豸���ƣ�Ϊ����ʹ��Ĭ�ϵĴ����豸
 * �����ܣ�ͨ����������ַ���"hello RT-Thread!"��Ȼ���λ���������ַ�
*/

#include <rtthread.h>
#include "rtdevice.h"
#include <string.h>

#define SAMPLE_UART_NAME       "uart2"

char *arr[4] = {
	"<G>ˮλ��\r\n",
	"<G>ˮλ��\r\n",
	"<G>ˮλ��\r\n",
	"<G>��������\r\n",
};

/* ���ڽ�����Ϣ���ź��� */
static struct rt_semaphore rx_sem;
static rt_device_t serial;

/* �������ݻص����� */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

int uart2_Init(void)
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    char str[] = "<G>���\r\n";

    rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);
  
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

    /* ��ʼ���ź��� */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* ���жϽ��ռ���ѯ����ģʽ�򿪴����豸 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_input);
    /* �����ַ��� */
    rt_device_write(serial, 0, str, (sizeof(str) - 1));

    return ret;
}
/* ������ msh �����б��� */
//INIT_APP_EXPORT(uart_sample);


int Audio_say(void)
{
	for(int i=0; i<4; i++)
	{
		rt_device_write(serial, 0, arr[i], strlen(arr[i]));
		rt_thread_mdelay(1000);
	}
}
MSH_CMD_EXPORT(Audio_say, Audio_say example);