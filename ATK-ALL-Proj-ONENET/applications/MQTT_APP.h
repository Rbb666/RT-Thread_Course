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

#define MQTT_Uri    	"tcp://8.141.59.93:1883"     		// MQTT服务器的地址和端口号
#define ClientId    	"751061401"                     // ClientId需要唯一
#define UserName    	"rb"                        		// 用户名
#define PassWord    	"123456"                        // 用户名对应的密码
#define RT_SUB_TOPIC  "topic_sub"                     // 订阅的TOPIC
#define RT_PUB_TOPIC  "topic_pub"                     // 发布的TOPIC

#define EVENT_TH_FLAG 		(1 << 3)
#define EVENT_ADC_FLAG 		(1 << 4)

#endif

