/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-08     obito0   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

/* defined the LED0 pin: PB5 */
#define LED0_PIN    GET_PIN(B, 5)
/* defined the LED1 pin: PE5 */
#define LED1_PIN    GET_PIN(E, 5)

#define EVENT_TH_FLAG 		(1 << 3)
#define EVENT_ADC_FLAG 		(1 << 4)

ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;
//
static void MX_ADC3_Init(void);
static void MX_DMA_Init(void);
float Get_Light_Value(void);
static void ADC_th(void *pram);
//
extern rt_event_t Sensor_event;			//传感器采集事件
//
static uint16_t adc_data;
float light_value;
//
int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

    rt_pin_write(LED0_PIN, PIN_HIGH);
    rt_pin_write(LED1_PIN, PIN_HIGH);

    return RT_EOK;
}

static void Thread_INT_th(void *p)
{
    //创建adc线程
    rt_thread_t adc_thread = rt_thread_create("adc_th", ADC_th, RT_NULL,
                             256, RT_THREAD_PRIORITY_MAX / 2 + 3, 20);

    if(adc_thread != RT_NULL)
    {
        rt_thread_startup(adc_thread);
    }

}

int Thread_init(void)
{
    rt_thread_t thread = rt_thread_create("INIT", Thread_INT_th, RT_NULL,
                                          512, RT_THREAD_PRIORITY_MAX / 2, 20);

    if(thread == RT_NULL)
    {
        return RT_ERROR;
    }

    rt_thread_startup(thread);

    return RT_EOK;
}
INIT_APP_EXPORT(Thread_init);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ADC_th(void *pram)
{
    MX_DMA_Init();
    MX_ADC3_Init();
    HAL_ADC_Start(&hadc3);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t*)&adc_data, (uint32_t)1);

    while(1)
    {
        Get_Light_Value();

        //发送ADC采集完毕事件
        rt_event_send(Sensor_event, EVENT_ADC_FLAG);

        rt_thread_mdelay(200);
    }
}

float Get_Light_Value(void)
{
    light_value = adc_data * 3.3f / 4096;
    return light_value;
    //rt_kprintf("light value is:%d.%02d\r\n",light_value/100,light_value%100);
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

    /* USER CODE BEGIN ADC3_Init 0 */

    /* USER CODE END ADC3_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC3_Init 1 */

    /* USER CODE END ADC3_Init 1 */
    /** Common config
    */
    hadc3.Instance = ADC3;
    hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc3.Init.ContinuousConvMode = ENABLE;
    hadc3.Init.DiscontinuousConvMode = DISABLE;
    hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc3.Init.NbrOfConversion = 1;

    if (HAL_ADC_Init(&hadc3) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* USER CODE BEGIN ADC3_Init 2 */

    /* USER CODE END ADC3_Init 2 */

}


/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

}

