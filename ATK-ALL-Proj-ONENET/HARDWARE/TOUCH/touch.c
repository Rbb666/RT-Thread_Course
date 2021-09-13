#include "touch.h"
#include "stdlib.h"
#include "math.h"

_m_tp_dev tp_dev =
{
    TP_Init,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

//��������ʼ��
//����ֵ:0,û�н���У׼
//       1,���й�У׼
uint8_t TP_Init(void)
{
    if(lcddev.id == 0X5510)				//4.3����ݴ�����
    {
        if(GT9147_Init() == 0)			//��GT9147
        {
            tp_dev.scan = GT9147_Scan;	//ɨ�躯��ָ��GT9147������ɨ��
        }

        tp_dev.touchtype |= 0X80;			//������
        tp_dev.touchtype |= lcddev.dir & 0X01; //������������
        return 0;
    }
		return 1;
}
