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

//触摸屏初始化
//返回值:0,没有进行校准
//       1,进行过校准
uint8_t TP_Init(void)
{
    if(lcddev.id == 0X5510)				//4.3寸电容触摸屏
    {
        if(GT9147_Init() == 0)			//是GT9147
        {
            tp_dev.scan = GT9147_Scan;	//扫描函数指向GT9147触摸屏扫描
        }

        tp_dev.touchtype |= 0X80;			//电容屏
        tp_dev.touchtype |= lcddev.dir & 0X01; //横屏还是竖屏
        return 0;
    }
		return 1;
}
