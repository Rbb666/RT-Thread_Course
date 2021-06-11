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

#define MQTT_Uri    	"tcp://xxx.xxx.xxx:1883"     		// MQTT服务器的地址和端口号
#define ClientId    	"751061401"                     // ClientId需要唯一
#define UserName    	"rb"                        		// 用户名
#define PassWord    	"123456"                        // 用户名对应的密码
#define RT_SUB_TOPIC  "topic_sub"                     // 订阅的TOPIC
#define RT_PUB_TOPIC  "topic_pub"                     // 发布的TOPIC


#define ONENET_INFO_DEVID 		"718305986"
#define ONENET_INFO_AUTH 			"Gate"
#define ONENET_INFO_APIKEY 		"99=HmhFBlg9=e3Snu8x25qRVfgU="
#define ONENET_INFO_PROID 		"426836"
#define ONENET_MASTER_APIKEY  "UvlcNUlBjs=URNWNS9t=atUbybc="
#define ONENET_SERVER_URL     "tcp://183.230.40.39:6002"

#define ONENET_INFO_DEVID_LEN          16
#define ONENET_INFO_APIKEY_LEN         32
#define ONENET_INFO_PROID_LEN          16
#define ONENET_INFO_AUTH_LEN           64
#define ONENET_INFO_NAME_LEN           64
#define ONENET_INFO_URL_LEN            32

#define ONENET_DATASTREAM_NAME_MAX     32

#define EVENT_TH_FLAG 		(1 << 3)
#define EVENT_ADC_FLAG 		(1 << 4)

struct rt_onenet_info
{
    char device_id[ONENET_INFO_DEVID_LEN];
    char api_key[ONENET_INFO_APIKEY_LEN];

    char pro_id[ONENET_INFO_PROID_LEN];
    char auth_info[ONENET_INFO_AUTH_LEN];

    char server_uri[ONENET_INFO_URL_LEN];

};
typedef struct rt_onenet_info *rt_onenet_info_t;
static rt_err_t onenet_get_info(void);

#endif

