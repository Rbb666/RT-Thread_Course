#ifndef __MQTT_APP_H__
#define __MQTT_APP_H__

#include <rtthread.h>
#include <rtdevice.h>
#include "UART_APP.h"
#include <string.h>

#include "FileSystem.h"
#include <at_device_esp8266.h>
#include <cjson.h>
#include "paho_mqtt.h"

#define MQTT_Uri    	"tcp://8.141.59.93:1883"     		// MQTT�������ĵ�ַ�Ͷ˿ں�
#define ClientId    	"751061401"                     // ClientId��ҪΨһ
#define UserName    	"rb"                        		// �û���
#define PassWord    	"123456"                        // �û�����Ӧ������
#define RT_SUB_TOPIC  "topic_sub"                     // ���ĵ�TOPIC
#define RT_PUB_TOPIC  "topic_pub"                     // ������TOPIC

#define EVENT_TH_FLAG 		(1 << 3)
#define EVENT_ADC_FLAG 		(1 << 4)

#endif

