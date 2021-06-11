# 玩转RT-Thread系列教程(1)--线程的使用

## 一、什么是线程

我们或许都听说过多线程的概念，其实在多CUP处理器上才真正的存在多线程的概念，每个CPU同时运行处理多个不同的任务。

那在我们的单核CPU的单片机上如何使用“多线程”来处理同一时刻请求的不同任务，来做到”同时“进行呢？



这个时候就需要引入线程管理了。在多线程操作系统中，需要开发人员把一个复杂的应用分解成多个小的、可调度的、

序列化的程序单元。而在 RT-Thread 中，与上述子任务对应的程序实体就是线程。

RT-Thread 的线程调度器是抢占式的，主要的工作就是从就绪线程列表中查找最高优先级线程，保证最高优先级的线程能够被运行，

最高优先级的任务一旦就绪，总能得到 CPU 的使用权。



![来源RT-Thread文档中心](https://img-blog.csdnimg.cn/20210531103609310.png)



## 二、线程的分类

**线程分为五种状态**：初始------->就绪------>运行------>挂起------>关闭

**空闲线程**：优先级最低，永远为就绪态，不被挂起。 	用处：回收被删除线程资源（回收僵尸线程）

当线程**优先级相同**时，采用**时间片轮转**方式调度，单位一个时钟节拍

比如：A：10，B：5，那么A线程执行10个节拍，B线程执行5个节拍

---

**rt_thread_yield()**：当前线程被换出，**相同优先级**的下一个就绪线程将被执行。

**rt_schedule()**：当前线程并不一定被换出，而是在系统中选取就绪的优先级最高的线程执行。

---

## 三、线程的创建

> 创建线程分为两种方式：1、动态创建线程	2、静态创建线程
>
> 两种方式各有优缺点，使用分具体场合。接下来我将具体分析以下两种创建线程方式。

### 3.1、创建线程(静态)

> 占用RAM空间(RW/ZI 空间)，用户分配栈空间和线程句柄

优点：运行时不需要动态分配内存，运行时效率较高，实时性较好，

缺点：内存不能被释放，只能使用 **rt_thread_detach()** 函数将该线程控制块从对象管理器中脱离。

```c
static rt_uint8_t thread1_stack[512];   //线程栈
static struct rt_thread thread1;			  //线程控制块
rt_thread_init(&thread1,							  //线程handle
               "thread1",					 	    //线程名称
               thread1_entry,					  //线程入口函数
               RT_NULL,							    //线程入口参数
               &thread1_stack[0],       //线程栈地址
               sizeof(thread1_stack),	  //线程栈大小
               15, 	 								    //线程优先级
               5);			 							  //线程时间片
rt_thread_startup(&thread2);					  //线程进入就绪态
```

---

### 3.2、创建线程(动态)

> 依赖与内存堆管理器，系统自动从动态内存堆分配栈空间

优点：创建方便，参数比较静态简便，内存可以由用户释放，调用 **rt_thread_delete()** 函数就会将这段申请的内存空间重新释放到内存堆中。

缺点：运行时需要动态分配内存，效率没有静态方式高，

```C
static rt_thread_t thread_id = RT_NULL;
thread_id = rt_thread_create("dynamic_th",    //名称
                              dynamic_entry,  //线程代码
                              RT_NULL,        //参数
                              1024,           //栈大小
                              15,             //优先级
                              20);            //时间片
if (thread_id != RT_NULL)
   rt_thread_startup(thread_id);					 	 //线程进入就绪态
else
   rt_kprintf("dynamic_thread create failure\n");
return RT_EOK;                                                          
```

## 四、创建线程示例

使用动态+静态方式，创建两个线程

```c
//动态线程任务
static void dynamic_entry(void *param)
{
		static int cnt = 0;
	
    while (++cnt)
    {
				rt_kprintf("dynamic_thread is run:%d\n",cnt);
        rt_thread_mdelay(500);
    }
}

//静态线程任务
static void static_entry(void *param)
{
		static int cnt = 0;
	
    while (++cnt)
    {
				rt_kprintf("static_thread is run:%d\n",cnt);
        rt_thread_mdelay(500);
    }
}

static rt_uint8_t thread1_stack[512];   					//线程栈
static struct rt_thread static_thread;			 			//线程控制块
int thread_sample(void)
{
    static rt_thread_t thread_id = RT_NULL;
    thread_id = rt_thread_create("dynamic_th",    //名称
                                 dynamic_entry,   //线程代码
                                 RT_NULL,         //参数
                                 1024,            //栈大小
                                 15,              //优先级
                                 20);             //时间片

    if (thread_id != RT_NULL)
        rt_thread_startup(thread_id);					 	  //线程进入就绪态
    else
        rt_kprintf("dynamic_thread create failure\n");

		rt_thread_init(&static_thread,						 //线程handle
									 "static_thread",						 //线程名称
									 static_entry,					 		 //线程入口函数
									 RT_NULL,							    	 //线程入口参数
									 &thread1_stack[0],       	 //线程栈地址
									 sizeof(thread1_stack),	  	 //线程栈大小
									 15, 	 								   		 //线程优先级
									 5);			 							     //线程时间片
		rt_thread_startup(&static_thread);				 //线程进入就绪态		
    return ret;								 
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_sample, thread sample);
```

| **初始化顺序** | **宏接口**                | **描述**                                     |
| -------------- | ------------------------- | -------------------------------------------- |
| 1              | INIT_BOARD_EXPORT(fn)     | 非常早期的初始化，此时调度器还未启动         |
| 2              | INIT_PREV_EXPORT(fn)      | 主要是用于纯软件的初始化、没有太多依赖的函数 |
| 3              | INIT_DEVICE_EXPORT(fn)    | 外设驱动初始化相关，比如网卡设备             |
| 4              | INIT_COMPONENT_EXPORT(fn) | 组件初始化，比如文件系统或者 LWIP            |
| 5              | INIT_ENV_EXPORT(fn)       | 系统环境初始化，比如挂载文件系统             |
| 6              | INIT_APP_EXPORT(fn)       | 应用初始化，比如 GUI 应用                    |

编译、下载：

![](https://img-blog.csdnimg.cn/20210531111134841.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

打开串口调试助手，输入thread_sample命令，可以看出我们的两个线程以及创建并且正常运行。

---

# 玩转RT-Thread系列教程(2)--定时器的使用

---

> 定时器有**硬件定时器**和**软件定时器**之分

---

## 一、软件定时器



> **软件定时器**是由操作系统提供的一类系统接口，它构建在硬件定时器基础之上，使系统能够提供不受数目限制的定时器服务。



RT-Thread 操作系统提供软件实现的定时器，以时钟节拍（OS Tick）的时间长度为单位，即定时数值必须是 OS Tick 的整数倍，例如一个 OS Tick 是 10ms，那么上层软件定时器只能是 10ms，20ms，100ms 等，而不能定时为 15ms。RT-Thread 的定时器也基于系统的节拍，提供了基于节拍整数倍的定时能力。

### 1.1、软件定时器示例

功能：创建两个软件定时器，一个是单次定时器，另一个是周期型定时器，分别在定时器超时函数中打印信息。

```c
/* 定时器的控制块 */
static rt_timer_t timer1;
static rt_timer_t timer2;
static int cnt = 0;

/* 定时器 1 超时函数 */
static void timeout1(void *parameter)
{
    rt_kprintf("periodic timer is timeout %d\n", cnt);

    /* 运行第 10 次，停止周期定时器 */
    if (cnt++>= 9)
    {
        rt_timer_stop(timer1);
        rt_kprintf("periodic timer was stopped! \n");
    }
}

/* 定时器 2 超时函数 */
static void timeout2(void *parameter)
{
    rt_kprintf("one shot timer is timeout\n");
}

int timer_sample(void)
{
    /* 创建定时器 1  周期定时器 */
    timer1 = rt_timer_create("timer1", timeout1,
                             RT_NULL, 10,
                             RT_TIMER_FLAG_PERIODIC);

    /* 启动定时器 1 */
    if (timer1 != RT_NULL) rt_timer_start(timer1);

    /* 创建定时器 2 单次定时器 */
    timer2 = rt_timer_create("timer2", timeout2,
                             RT_NULL,  30,
                             RT_TIMER_FLAG_ONE_SHOT);

    /* 启动定时器 2 */
    if (timer2 != RT_NULL) rt_timer_start(timer2);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(timer_sample, timer sample);
```

### 1.2、编译、下载、验证

![](https://img-blog.csdnimg.cn/20210531142131614.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

可以看出，单次定时器在30ms时便完成了超时任务，而周期定时器只有在手动关闭时才会停止。



## 二、HWTIMER定时器



> **硬件定时器**是芯片本身提供的定时功能。一般是由外部晶振提供给芯片输入时钟，芯片向软件模块提供一组配置寄存器，接受控制输入，到达设定时间值后芯片中断控制器产生时钟中断。硬件定时器的精度一般很高，可以达到纳秒级别，并且是中断触发方式。



### 2.1、Cubemx使能基本定时器

![Cubemx](https://img-blog.csdnimg.cn/20210531134856211.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.2、使用env配置工程

![](https://img-blog.csdnimg.cn/20210531114334484.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210531114346942.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.3、使能基本定时器

![](https://img-blog.csdnimg.cn/20210531114400650.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​		经过查看f1的参考手册，我们可以看见TIM2、3、4、5是基本定时器

![](https://img-blog.csdnimg.cn/2021053111454641.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210531122515420.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.4、生成工程

> 输入命令scons --target=mdk5

![](https://img-blog.csdnimg.cn/20210531114441552.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### **2.5、示例代码**

测试代码如下，实验功能为：设置定时器3为周期性定时器，设置定时器超时值为1s，每1s将led电平反转一次。

```c
#define HWTIMER_DEV_NAME   "timer3"     /* 定时器名称 */

/* 定时器超时回调函数 */
static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("this is hwtimer timeout callback fucntion!\n");
    rt_kprintf("tick is :%d !\n", rt_tick_get());

    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);

    rt_kprintf("LED1 AND LED2 TogglePin!\n");

    return 0;
}

static int hwtimer_sample(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
    rt_device_t hw_dev = RT_NULL;   /* 定时器设备句柄 */
    rt_hwtimer_mode_t mode;         /* 定时器模式 */

    /* 查找定时器设备 */
    hw_dev = rt_device_find(HWTIMER_DEV_NAME);

    if (hw_dev == RT_NULL)
    {
        rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
        return RT_ERROR;
    }

    /* 以读写方式打开设备 */
    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);

    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
        return ret;
    }

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(hw_dev, timeout_cb);

    /* 设置模式为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);

    if (ret != RT_EOK)
    {
        rt_kprintf("set mode failed! ret is :%d\n", ret);
        return ret;
    }

    /* 设置定时器超时值为1s并启动定时器 */
    timeout_s.sec = 1;      /* 秒 */
    timeout_s.usec = 0;     /* 微秒 */

    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        rt_kprintf("set timeout value failed\n");
        return RT_ERROR;
    }

    return ret;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(hwtimer_sample, hwtimer sample);
```

6.编译、下载、验证

![](https://img-blog.csdnimg.cn/20210531140117385.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

下载到开发板，我们可以观察到led1和led2以1s的频率闪烁了。

---

# 玩转RT-Thread系列教程(3)--消息邮箱的使用



## 一、什么是消息邮箱

> 邮箱服务是实时操作系统中一种典型的线程间通信方法。

RT-Thread 操作系统的邮箱用于线程间通信，特点是**开销比较低，效率较高**。邮箱中的每一封邮件只能容纳固定的 4 字节内容

（针对 32 位处理系统，指针的大小即为 4 个字节，所以一封邮件恰好能够容纳一个指针）。

通常来说，邮件收取过程可能是阻塞的，这取决于邮箱中是否有邮件，以及收取邮件时设置的超时时间。

---

**1.发送邮件**

当一个线程向邮箱发送邮件时，如果邮箱没满，将把邮件复制到邮箱中。如果邮箱已经满了，发送线程可以设置超时时间，

选择等待挂起或直接返回 - RT_EFULL。如果发送线程选择挂起等待，那么当邮箱中的邮件被收取而空出空间来时，等待挂起的发送线程将被唤醒继续发送。

---

**2.接收邮件**

当一个线程从邮箱中接收邮件时，如果邮箱是空的，接收线程可以选择是否等待挂起直到收到新的邮件而唤醒，或可以设置超时时间。

当达到设置的超时时间，邮箱依然未收到邮件时，这个选择超时等待的线程将被唤醒并返回 - RT_ETIMEOUT。如果邮箱中存在邮件，

那么接收线程将复制邮箱中的 4 个字节邮件到接收缓存中。

---



## 二、消息邮箱的使用

介绍完了消息邮箱，那么消息邮箱的使用场合是什么呢？邮箱是一种简单的线程间消息传递方式，特点是开销比较低，效率较高。邮箱具备一定的存储功能，能够缓存一定数量的邮件数。

接下来让我们用一段示例代码演示以下消息邮箱的使用：

### 2.1、功能设计

线程1会向线程2发送消息，线程2接收到消息存放到邮箱中并串口输出该消息，同时发送消息给线程1，线程1接收到消息存放到邮箱中并串口输出该消息。当运行完2次发送任务时，删除二者邮箱，并串口输出“Mailboxes demo finsh”。

### 2.2、代码设计

```C
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

/* defined the LED0 pin: PB5 */
#define LED0_PIN    GET_PIN(B, 5)
/* defined the LED1 pin: PE5 */
#define LED1_PIN    GET_PIN(E, 5)

/* 邮箱控制块 */
static struct rt_mailbox mb1;
/* 用于放邮件的内存池 */
static char mb_pool1[128];

/* 邮箱控制块 */
static struct rt_mailbox mb2;
/* 用于放邮件的内存池 */
static char mb_pool2[128];

static rt_uint8_t thread2_stack[512];   					//线程栈
static struct rt_thread static_thread;			 			//线程控制块

static char mb_str1[] = "thread1 send information";
static char mb_str2[] = "thread2 send information";

int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

    rt_pin_mode(KEY0_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY1_PIN, PIN_MODE_INPUT_PULLUP);
}

//thread1
static void dynamic_entry(void *param)
{
    char *str;
    static rt_uint8_t cnt;

    while (cnt < 2)
    {
        cnt++;

        /* 发送 mb_str1 地址到邮箱中 */
        rt_mb_send(&mb2, (rt_uint32_t)&mb_str1);
        rt_kprintf("@thread1 send mb to thread2\r\n");

        /* 从邮箱中收取邮件 */
        if (rt_mb_recv(&mb1, (rt_ubase_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread1: get a mail from mailbox1, the content:%s\r\n", str);
            rt_thread_mdelay(50);
        }

        rt_thread_mdelay(50);
    }

    /* 执行邮箱对象脱离 */
    if(rt_mb_detach(&mb1) != RT_EOK)
        rt_kprintf("mb1 detach fail\r\n");
}

//thread2
static void static_entry(void *param)
{
    char *str;
    static rt_uint8_t cnt;

    while (cnt < 2)
    {
        cnt++;

        /* 从邮箱中收取邮件 */
        if (rt_mb_recv(&mb2, (rt_ubase_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread2: get a mail from mailbox2, the content:%s\r\n", str);
            rt_thread_mdelay(50);
            rt_kprintf("@thread2 send mb to thread1\r\n");

            /* 发送 mb_str2 地址到邮箱中 */
            rt_mb_send(&mb1, (rt_uint32_t)&mb_str2);
        }

        rt_thread_mdelay(50);
    }

    if(rt_mb_detach(&mb2) != RT_EOK)
        rt_kprintf("mb2 detach fail\r\n");

    rt_kprintf("Mailboxes demo finsh\r\n");
}

int MailBox_demo(void)
{

    rt_err_t result1, result2;

    /* 初始化一个 mailbox */
    result1 = rt_mb_init(&mb1,
                         "mbt1",                      /* 名称是 mbt */
                         &mb_pool1[0],                /* 邮箱用到的内存池是 mb_pool */
                         sizeof(mb_pool1) / 4,        /* 邮箱中的邮件数目，因为一封邮件占 4 字节 */
                         RT_IPC_FLAG_FIFO);           /* 采用 FIFO 方式进行线程等待 */

    if (result1 != RT_EOK)
    {
        rt_kprintf("init mailbox1 failed.\r\n");
        return -1;
    }

    /* 初始化一个 mailbox */
    result2 = rt_mb_init(&mb2,
                         "mbt2",                      /* 名称是 mbt */
                         &mb_pool2[0],                /* 邮箱用到的内存池是 mb_pool */
                         sizeof(mb_pool2) / 4,        /* 邮箱中的邮件数目，因为一封邮件占 4 字节 */
                         RT_IPC_FLAG_FIFO);           /* 采用 FIFO 方式进行线程等待 */

    if (result2 != RT_EOK)
    {
        rt_kprintf("init mailbox2 failed.\r\n");
        return -1;
    }

    //
    static rt_thread_t thread_id = RT_NULL;
    thread_id = rt_thread_create("thread1",    		//名称
                                 dynamic_entry,   //线程代码
                                 RT_NULL,         //参数
                                 512,            	//栈大小
                                 14,              //优先级
                                 20);             //时间片

    if (thread_id != RT_NULL)
        rt_thread_startup(thread_id);					 	  //线程进入就绪态
    else
        rt_kprintf("dynamic_thread create failure\r\n");

    //
    rt_thread_init(&static_thread,							//线程handle
                   "thread2",										//线程名称
                   static_entry,					  		//线程入口函数
                   RT_NULL,							    		//线程入口参数
                   &thread2_stack[0],       		//线程栈地址
                   sizeof(thread2_stack),	  		//线程栈大小
                   15, 	 								    		//线程优先级
                   10);			 							  		//线程时间片
    rt_thread_startup(&static_thread);					//线程进入就绪态

    return RT_EOK;
}

MSH_CMD_EXPORT(MailBox_demo, MailBox_demo);
```

3.编译、下载、查看结果

![](https://img-blog.csdnimg.cn/20210531184451768.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



​			通过串口数据我们不难发现，消息邮箱的运行机制，一方发送一方接收。

---

# 玩转RT-Thread系列教程(4)--消息队列的使用



## 一、什么是消息队列



> 消息队列是另一种常用的线程间通讯方式，是邮箱的扩展。可以应用在多种场合：线程间的消息交换、使用串口接收不定长数据等。



## 二、消息队列的工作机制

消息队列能够接收来自线程或中断服务例程中不固定长度的消息，并把消息缓存在自己的内存空间中。

其他线程也能够从消息队列中读取相应的消息，而当消息队列是空的时候，可以挂起读取线程。

当有新的消息到达时，挂起的线程将被唤醒以接收并处理消息。消息队列是一种异步的通信方式。



> 当有多个消息发送到消息队列时，通常将先进入消息队列的消息先传给线程，也就是说，线程先得到的是最先进入消息队列的消息，即先进先出原则 (FIFO)。

![RT-Thread官方文档](https://img-blog.csdnimg.cn/20210531190406392.png)



## 三、消息队列的使用

> 这次的实验示例我采用串口DMA接收来做，为后续我们的读取485温湿度传感器数据教程打下基础。



### 3.1、串口DMA接收原理

当串口接收到一批数据后会调用接收回调函数，接收回调函数会把此时缓冲区的数据大小通过消息队列发送给等待的数据处理线程。

线程获取到消息后被激活，并读取数据。一般情况下 DMA 接收模式会结合 DMA 接收完成中断和串口空闲中断完成数据接收。

![来自RT-Thread官方文档](https://img-blog.csdnimg.cn/20210531190804383.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

上图展示了DMA接收到串口消息的全部过程，接下来让我们配置以下板载的串口吧

### 3.2、串口外设的配置

​			**3.2.1、查看串口原理图；串口2接在了板子上的PA2,PA3上面**

![](https://img-blog.csdnimg.cn/20210531191804223.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210531191834155.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



​		**3.2.2、打开Cubemx配置串口外设**

![](https://img-blog.csdnimg.cn/20210531192058309.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



​		**3.2.3、打开env配置串口外设，添加DMA方式接收**

![](https://img-blog.csdnimg.cn/20210531192134831.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​		**3.2.4、使用命令，生成代码**

![](https://img-blog.csdnimg.cn/20210531192144578.png)

​		**3.2.5、编译**

![](https://img-blog.csdnimg.cn/20210531192336924.png)

​		**3.2.6、添加UART_APP文件**

![](https://img-blog.csdnimg.cn/20210531192512618.png)

![](https://img-blog.csdnimg.cn/20210531192554279.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



​		**3.2.7、解决DMA接收出现分包的小bug**

​				在drv_usart.c中屏蔽以下字段

![](https://img-blog.csdnimg.cn/20210531193322310.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​		3.2.8、添加业务代码

```c
#include <rtthread.h>

#include "UART_APP.h"
#define SAMPLE_UART_NAME       "uart2"      /* 串口设备名称 */

/* 串口接收消息结构*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};
/* 串口设备句柄 */
static rt_device_t serial;
/* 消息队列控制块 */
static struct rt_messagequeue rx_mq;

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
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);

        if (result == RT_EOK)
        {
            /* 从串口读取数据*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rx_buffer[rx_length] = '\0';
            /* 通过串口设备 serial 输出读取到的消息 */
            rt_device_write(serial, 0, rx_buffer, rx_length);
            /* 打印数据 */
            rt_kprintf("%s\n", rx_buffer);
        }
    }
}

static int uart_dma_sample(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    static char msg_pool[256];

    if (argc == 2)
    {
        rt_strncpy(uart_name, argv[1], RT_NAME_MAX);
    }
    else
    {
        rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);
    }

    /* 查找串口设备 */
    serial = rt_device_find(uart_name);

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
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);

    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(uart_dma_sample, uart device dma sample);

```

![](https://img-blog.csdnimg.cn/20210531200224844.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210531194338191.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210531200159134.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​		3.2.9、编译、下载、查看结果

![](https://img-blog.csdnimg.cn/20210531193711597.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

---



---



# 玩转RT-Thread系列教程(5)--MultiButton

## 一、MultiButton介绍

### 1.1、面向对象思想

MultiButton中每个按键都抽象为了一个按键对象，每个按键对象是独立的，系统中所有的按键对象使用单链表串起来。

![](https://img-blog.csdnimg.cn/20210531210141728.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

其中在变量后面跟冒号的语法称为位域，使用位域的优势是节省内存。

第一次插入时，因为head_hanler 为 NULL，所以只需要执行while之后的代码.

![](https://img-blog.csdnimg.cn/20201223182400553.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

如果再插入一个buuton2按键对象:
![](https://img-blog.csdnimg.cn/20201223182440384.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 1.2、状态机处理思想

1. 读取当前引脚状态

![](https://img-blog.csdnimg.cn/20201223182632103.png)

2. 读取之后，判断当前状态机的状态，如果有功能正在执行（state不为0），则按键对象的tick值加1（后续一切功能的基础）

![](https://img-blog.csdnimg.cn/20201223182749608.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

3. 按键消抖（连续读取3次，15ms，如果引脚状态一直与之前不同，则改变按键对象中的引脚状态）

![](https://img-blog.csdnimg.cn/20201223203813880.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

4. 状态机

![](https://img-blog.csdnimg.cn/20201223203917916.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​																																			**---MultiButton参考Mculover666教程,感谢大佬让我学习到很多。**

## 二、添加软件包

### 2.1、menuconfig

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531200800412.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531200809941.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531200819848.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531200827415.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.2、更新下载软件包

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531200859707.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.3、生成mdk工程

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531200913804.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.4、添加button_app.c

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531205450352.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



## 三、示例使用

### 3.1、查看电路原理图

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531213607405.png)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210531213540630.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 3.2、在Button_APP.c中添加按键驱动

![](https://img-blog.csdnimg.cn/20210531213935322.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

```c
#include "button_app.h"

extern struct key_state_type key0;
extern struct key_state_type key1;

static Button_t button_0;
static Button_t button_1;

//read button 0 level
uint8_t button0_read_level(void)
{
    return rt_pin_read(KEY_0);
}

//button 0 down callback
void button0_down_callback(void *btn)
{
    key0.down_state = KEY_PRESS;
}

// button 0 long callback
void button0_double_callback(void *btn)
{
    key0.double_state = KEY_PRESS;
}

// button 0 long callback
void button0_long_callback(void *btn)
{
    key0.long_state = KEY_PRESS;
}

//read button 1 level
uint8_t button1_read_level(void)
{
    return rt_pin_read(KEY_1);
}
//button1 down callback
void button1_down_callback(void *btn)
{
    key1.down_state = KEY_PRESS;
}
// button1 long callback
void button1_double_callback(void *btn)
{
    key1.double_state = KEY_PRESS;
}
// button 1 long callback
void button1_long_callback(void *btn)
{
    key1.long_state = KEY_PRESS;
}

void key_process(void)
{
    Button_Process();
}

int	key_init(void)
{
    //button gpio init
    rt_pin_mode(KEY_0, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_1, PIN_MODE_INPUT_PULLUP);

    //button create
    Button_Create("button_0", &button_0, button0_read_level, PIN_LOW);
    Button_Create("button_1", &button_1, button1_read_level, PIN_LOW);

    //set button callback
    Button_Attach(&button_0, BUTTON_DOWM, button0_down_callback);
    Button_Attach(&button_0, BUTTON_DOUBLE, button0_double_callback);
    Button_Attach(&button_0, BUTTON_LONG, button0_long_callback);

    Button_Attach(&button_1, BUTTON_DOWM, button1_down_callback);
    Button_Attach(&button_1, BUTTON_DOUBLE, button1_double_callback);
    Button_Attach(&button_1, BUTTON_LONG, button1_long_callback);

    return RT_EOK;
}

INIT_APP_EXPORT(key_init);

```

### 3.3、按键功能编写

```C
//定义key 控制块
struct key_state_type key0 = { 0 };
struct key_state_type key1 = { 0 };

int main(void)
{
    while(1)
    {
        key_process();
				
        if(key0.double_state == KEY_PRESS)
        {
            rt_kprintf("key0 is double_pass\n");
						key0.double_state = KEY_RELEASE;
        }
        else if (key0.long_state == KEY_PRESS)
        {
            rt_kprintf("key0 is long_pass\n");
						key0.long_state = KEY_RELEASE;
        }
				
        if(key1.double_state == KEY_PRESS)
        {
            rt_kprintf("key1 is double_pass\n");
						key1.double_state = KEY_RELEASE;
        }
        else if (key1.long_state == KEY_PRESS)
        {
            rt_kprintf("key1 is long_pass\n");
						key1.long_state = KEY_RELEASE;
        }
				
				rt_thread_mdelay(10);
    }
}
```

### 3.4、编译、下载、查看

![](https://img-blog.csdnimg.cn/20210531214125965.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

可以看到我们的按键双击，长按释放事件已经得到响应。



---

# 玩转RT-Thread系列教程(6)--移植STemwin

## 一、STemwin介绍

在实际应用中我们时常需要制作 UI 界面来实现人机交互，简单的 UI 我们可以自己直接写代码，但是对于那些复杂的交互方式和界面自己写代码的话难度就会很大。为了方便开发我们可以使用第三方的 GUI 库来做 UI 界面设计，在 STM32 上最火的 GUI 库莫过于 UCGUI，而UCGUI 的高级版本就是 emWin，STemWin 是 SEGGER 授权给 ST 的 emWin 版本，ST 的芯片可以免费使用 STemWin，而且 STemWin 针对 ST 的芯片做了优化。

![](https://img-blog.csdnimg.cn/20210530165444500.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

## 二、添加软件包

### 2.1、进入env配置界面

​		menuconfig

![](https://img-blog.csdnimg.cn/20210530165512597.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.2、选择我们要选择的软件包

​		添加屏幕驱动（这里我使用的是正点原子4.3寸电容触摸屏--触摸芯片型号为GT9147）

![](https://img-blog.csdnimg.cn/20210530165543761.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210530165613241.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210530165633733.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.3、使能内存分配算法

![](https://img-blog.csdnimg.cn/20210530165654584.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/2021053016571326.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



### 2.4、添加IIC驱动

![](https://img-blog.csdnimg.cn/20210530165735781.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210530165751461.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​		经过对比引脚，我们开发板的触摸iic引脚分布在PB1、PF11上。



### 2.5、添加STemwin软件包

![](https://img-blog.csdnimg.cn/20210530165809864.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210530165825509.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.6、从仓库拉取软件包

![](https://img-blog.csdnimg.cn/20210530165846574.png)

### 2.7、生成MDK工程

![](https://img-blog.csdnimg.cn/20210530165904107.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​		打开工程可以看见已经生成了对应的工程



![](https://img-blog.csdnimg.cn/20210530165922294.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



## 三、驱动配置

### 3.1、查看电路原理图

​											SRAM和LCD对应的FSMC_NE3，FSMC_NE4

![](https://img-blog.csdnimg.cn/20210530165942885.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



![](https://img-blog.csdnimg.cn/20210530170001104.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



![](https://img-blog.csdnimg.cn/20210530170021840.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



### 3.2、使用Cubemx配置驱动

![](https://img-blog.csdnimg.cn/20210530170039582.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



![](https://img-blog.csdnimg.cn/20210530170054996.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 3.3、CRC配置

![](https://img-blog.csdnimg.cn/20210530170110815.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 3.4、添加SRAM驱动

​	打开STemwin官方实例的drv_lcd.c文件，正好正点原子屏幕是使用fsmc驱动的，所以我们只需简单配置即可。

![](https://img-blog.csdnimg.cn/20210530170126984.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

```c
//初始化外部SRAM
void FSMC_SRAM_Init(void)
{
    RCC->AHBENR |= 1 << 8;       //使能FSMC时钟
    RCC->APB2ENR |= 1 << 5;      //使能PORTD时钟
    RCC->APB2ENR |= 1 << 6;      //使能PORTE时钟
    RCC->APB2ENR |= 1 << 7;      //使能PORTF时钟
    RCC->APB2ENR |= 1 << 8;      //使能PORTG时钟

    //PORTD复用推挽输出
    GPIOD->CRH &= 0X00000000;
    GPIOD->CRH |= 0XBBBBBBBB;
    GPIOD->CRL &= 0XFF00FF00;
    GPIOD->CRL |= 0X00BB00BB;
    //PORTE复用推挽输出
    GPIOE->CRH &= 0X00000000;
    GPIOE->CRH |= 0XBBBBBBBB;
    GPIOE->CRL &= 0X0FFFFF00;
    GPIOE->CRL |= 0XB00000BB;
    //PORTF复用推挽输出
    GPIOF->CRH &= 0X0000FFFF;
    GPIOF->CRH |= 0XBBBB0000;
    GPIOF->CRL &= 0XFF000000;
    GPIOF->CRL |= 0X00BBBBBB;

    //PORTG复用推挽输出 PG10->NE3
    GPIOG->CRH &= 0XFFFFF0FF;
    GPIOG->CRH |= 0X00000B00;
    GPIOG->CRL &= 0XFF000000;
    GPIOG->CRL |= 0X00BBBBBB;

    //寄存器清零
    //bank1有NE1~4,每一个有一个BCR+TCR，所以总共八个寄存器。
    //这里我们使用NE3 ，也就对应BTCR[4],[5]。
    FSMC_Bank1->BTCR[4] = 0X00000000;
    FSMC_Bank1->BTCR[5] = 0X00000000;
    FSMC_Bank1E->BWTR[4] = 0X00000000;
    //操作BCR寄存器  使用异步模式,模式A(读写共用一个时序寄存器)
    //BTCR[偶数]:BCR寄存器;BTCR[奇数]:BTR寄存器
    FSMC_Bank1->BTCR[4] |= 1 << 12;    //存储器写使能
    FSMC_Bank1->BTCR[4] |= 1 << 4; //存储器数据宽度为16bit
    //操作BTR寄存器
    FSMC_Bank1->BTCR[5] |= 3 << 8; //数据保持时间（DATAST）为3个HCLK 4/72M=55ns(对EM的SRAM芯片)
    FSMC_Bank1->BTCR[5] |= 0 << 4; //地址保持时间（ADDHLD）未用到
    FSMC_Bank1->BTCR[5] |= 0 << 0; //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
    //闪存写时序寄存器
    FSMC_Bank1E->BWTR[4] = 0x0FFFFFFF;    //默认值
    //使能BANK1区域3
    FSMC_Bank1->BTCR[4] |= 1 << 0;
}

```

​		添加到LCD初始化下面

![](https://img-blog.csdnimg.cn/20210530170147697.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210530170202827.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



​						进入gt9147.c中，添加触摸屏初始化：

![](https://img-blog.csdnimg.cn/20210530170218685.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

```c
//复位引脚
#define GT9147_RST_PIN 91
//中断引脚--PF10 INT
#define GT9147_IRQ_PIN 90

int rt_hw_gt9147_port(void)
{
    struct rt_touch_config config;
    rt_uint8_t rst;

    rst = GT9147_RST_PIN;
    config.dev_name = "i2c1";
    config.irq_pin.pin = GT9147_IRQ_PIN;
    config.irq_pin.mode = PIN_MODE_INPUT_PULLDOWN;
    config.user_data = &rst;

    rt_hw_gt9147_init("gt", &config);

    return 0;
}
INIT_ENV_EXPORT(rt_hw_gt9147_port);
```

这样，我们的LCD和SRAM就配置好了。

### 3.5、编译、下载，打开串口助手

![](https://img-blog.csdnimg.cn/20210530170240878.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

输入list_device，可以看见lcd和gt触摸屏设备已经注册好了。

输入free，查看串口返回：

![](https://img-blog.csdnimg.cn/20210530170256240.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

串口返回1048528字节，1048528/1024=1,023.9k=1M，间接证明了我们开发板的SRAM的内存是1M。



## 四、STemwin配置

### **4.1、关于STemwin官方文件说明**



![](https://img-blog.csdnimg.cn/20210530170310229.png)

### **4.2、系统配置**

​							 在GUIConf.c中使用动态从SRAM中申请方式为STemwin分配512k的内存。

![](https://img-blog.csdnimg.cn/20210530170325955.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### **4.3、触摸屏移植**

​	配置**GUI_X_Touch_Analog.c** 文件：

**GUI_X_Touch_Analog.c** 文件中有四个函数 :**GUI_TOUCH_X_ActivateX()** ，**GUI_TOUCH_X_ActivateY()**， **GUI_TOUCH_X_MeasureX()**和 **GUI_TOUCH_X_MeasureY()**。其中前两个我们没有使用到 ，STemWin 真正调用**GUI_TOUCH_X_MeasureX**和**GUI_TOUCH_X_MeasureY**这两个函数来获取触摸屏按下时的X轴和Y轴AD值。

```c
void GUI_TOUCH_X_ActivateX(void)
{
}

void GUI_TOUCH_X_ActivateY(void)
{
}

int  GUI_TOUCH_X_MeasureX(void)
{
    int32_t xvalue;

    for (rt_uint8_t i = 0; i < info.point_num; i++)
    {
        if (read_data[i].event == RT_TOUCH_EVENT_DOWN || read_data[i].event == RT_TOUCH_EVENT_MOVE)
        {
            xvalue = read_data[i].x_coordinate;
        }
        else
        {
            xvalue = 0xffff;
        }
        return xvalue;
    }
}

int  GUI_TOUCH_X_MeasureY(void)
{
    int32_t yvalue;

    for (rt_uint8_t i = 0; i < info.point_num; i++)
    {
        if (read_data[i].event == RT_TOUCH_EVENT_DOWN || read_data[i].event == RT_TOUCH_EVENT_MOVE)
        {
            yvalue = read_data[i].y_coordinate;
        }
        else
        {
            yvalue = 0xffff;
        }
        return yvalue;
    }
}

static void gt9147_entry(void *parameter)
{
    rt_device_control(dev, RT_TOUCH_CTRL_GET_INFO, &info);

    read_data = (struct rt_touch_data *)rt_malloc(sizeof(struct rt_touch_data) * info.point_num);

    while (1)
    {
        //获取信号量
        rt_sem_take(gt9147_sem, RT_WAITING_FOREVER);
        //触摸屏是否按下
        rt_device_read(dev, 0, read_data, info.point_num);
        //开中断
        rt_device_control(dev, RT_TOUCH_CTRL_ENABLE_INT, RT_NULL);
    }
}

static rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    //释放信号量
    rt_sem_release(gt9147_sem);
    //关中断
    rt_device_control(dev, RT_TOUCH_CTRL_DISABLE_INT, RT_NULL);
    return 0;
}

/* Test function */
int gt9147_sample(const char *name, rt_uint16_t x, rt_uint16_t y)
{
    void *id;

    dev = rt_device_find(name);

    if (dev == RT_NULL)
    {
        rt_kprintf("can't find device:%s\n", name);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("open device failed!");
        return -1;
    }

    id = rt_malloc(sizeof(struct rt_touch_info));
    rt_device_control(dev, RT_TOUCH_CTRL_GET_ID, id);
    rt_uint8_t * read_id = (rt_uint8_t *)id;
    rt_kprintf("id = %d %d %d %d \n", read_id[0] - '0', read_id[1] - '0', read_id[2] - '0', read_id[3] - '0');

    rt_device_control(dev, RT_TOUCH_CTRL_SET_X_RANGE, &x);  /* if possible you can set your x y coordinate*/
    rt_device_control(dev, RT_TOUCH_CTRL_SET_Y_RANGE, &y);
    rt_device_control(dev, RT_TOUCH_CTRL_GET_INFO, id);
    rt_kprintf("range_x = %d \n", (*(struct rt_touch_info*)id).range_x);
    rt_kprintf("range_y = %d \n", (*(struct rt_touch_info*)id).range_y);
    rt_kprintf("point_num = %d \n", (*(struct rt_touch_info*)id).point_num);
    rt_free(id);

    gt9147_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);

    if (gt9147_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }

    rt_device_set_rx_indicate(dev, rx_callback);

    gt9147_thread = rt_thread_create("touch_lcd",
                                     gt9147_entry,
                                     RT_NULL,
                                     THREAD_STACK_SIZE,
                                     THREAD_PRIORITY,
                                     THREAD_TIMESLICE);

    if (gt9147_thread != RT_NULL)
        rt_thread_startup(gt9147_thread);

    return 0;
}

int touch_init(void)
{
    gt9147_sample("gt", 800, 480);
    return RT_EOK;
}
INIT_APP_EXPORT(touch_init);
```



​						最后修改 **LCDConf_FlexColor_Template.c** 文件中的 LCD_X_Config()函数，在最后加上触摸屏的校准（必须）



![](https://img-blog.csdnimg.cn/20210530170355532.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### **4.4、STemwin的“点灯仪式”**

既然我们已经移植并配置好了LCD以及触摸功能，那么就来测试一下我们的emwin是否工作正常吧

我们新建一个stemwin测试文件，测试代码如下：

```c
#include <rtthread.h>
#include "GUI.h"
#include <board.h>

#include "DIALOG.h"
//////////////////////////////////////////////////////////
//TOUCH任务
//设置任务优先级
#define TOUCH_TASK_PRIO				  15
//任务堆栈大小
#define TOUCH_STK_SIZE				  216

//EMWINDEMO任务
//设置任务优先级
#define EMWINDEMO_TASK_PRIO			18
//任务堆栈大小
#define EMWINDEMO_STK_SIZE			2048
//////////////////////////////////////////////////////////

#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x00)
#define ID_BUTTON_0 (GUI_ID_USER + 0x01)
#define ID_BUTTON_1 (GUI_ID_USER + 0x02)

//对话框资源表
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] =
{
    { FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_0, 0, 0, 800, 480, FRAMEWIN_CF_MOVEABLE, 0x64, 0 },
    { BUTTON_CreateIndirect, "Button", ID_BUTTON_0, 300, 122, 150, 50, 0, 0x0, 0 },
    { BUTTON_CreateIndirect, "Button", ID_BUTTON_1, 300, 251, 150, 50, 0, 0x0, 0 },
};

//对话框回调函数
static void _cbDialog(WM_MESSAGE * pMsg)
{
    WM_HWIN hItem;
    int     NCode;
    int     Id;

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
            //初始化对话框
            hItem = pMsg->hWin;
            FRAMEWIN_SetTitleHeight(hItem, 30);
            FRAMEWIN_SetText(hItem, "RB_RT-Thread STemwin demo");
            FRAMEWIN_SetFont(hItem, GUI_FONT_24_ASCII);
            FRAMEWIN_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
            FRAMEWIN_SetTextColor(hItem, 0x0000FFFF);

            //初始化BUTTON0
            hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
            BUTTON_SetFont(hItem, GUI_FONT_24_ASCII);
            BUTTON_SetText(hItem, "LED1");

            //初始化BUTTON1
            hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);
            BUTTON_SetText(hItem, "BEEP");
            BUTTON_SetFont(hItem, GUI_FONT_24_ASCII);
            break;

        case WM_NOTIFY_PARENT:
            Id    = WM_GetId(pMsg->hWinSrc);
            NCode = pMsg->Data.v;

            switch(Id)
            {
                case ID_BUTTON_0: //BUTTON_0的通知代码，控制LED1
                    switch(NCode)
                    {
                        case WM_NOTIFICATION_CLICKED:
                            break;
                        case WM_NOTIFICATION_RELEASED: //按钮被按下并释放
                            rt_kprintf("led on\n");
                            break;
                    }
                    break;

                case ID_BUTTON_1: //BUTTON_1的通知代码，控制BEEP
                    switch(NCode)
                    {
                        case WM_NOTIFICATION_CLICKED:
                            break;

                        case WM_NOTIFICATION_RELEASED:
                            rt_kprintf("beep on\n");
                            //LED1=~LED1;
                            break;
                    }
                    break;
            }
            break;
        default:
            WM_DefaultProc(pMsg);
            break;
    }
}

//创建一个对话框
WM_HWIN CreateFramewin(void)
{
    WM_HWIN hWin;
    //非阻塞式对话框
    hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
    return hWin;
}

//BUTTON按钮上显示位图
void Buttonbmp_Demo(void)
{
    WM_HWIN hWin;
    hWin = CreateFramewin();

    while(1)
    {
        GUI_Delay(10);
    }
}

static void Sys_CRC_Init(void)
{
    CRC_HandleTypeDef CrcHandle;
    CrcHandle.State = HAL_CRC_STATE_RESET;
    CrcHandle.Instance = CRC;
    HAL_CRC_Init(&CrcHandle);
}

//显示任务
static void STemwin_thread(void *param)
{
    Sys_CRC_Init();
    WM_SetCreateFlags(WM_CF_MEMDEV); 	//启动所有窗口的存储设备
    GUI_Init();
    //
    GUI_CURSOR_Show();
    //更换皮肤
    BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
    CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
    DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
    FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
    HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
    MENU_SetDefaultSkin(MENU_SKIN_FLEX);
    MULTIPAGE_SetDefaultSkin(MULTIPAGE_SKIN_FLEX);
    PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
    RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
    SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
    SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
    SPINBOX_SetDefaultSkin(SPINBOX_SKIN_FLEX);

    while (1)
    {
        Buttonbmp_Demo();
    }
}

//触摸任务
static void touch_thread(void *param)
{
    while(1)
    {
        GUI_TOUCH_Exec();
        rt_thread_mdelay(5);
    }
}

static int Gui_emwin_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("emwin",
                           STemwin_thread, RT_NULL,
                           EMWINDEMO_STK_SIZE,
                           EMWINDEMO_TASK_PRIO, 10);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(Gui_emwin_init);

static int Gui_touch_init(void)
{
    rt_thread_t btn_tid;
    btn_tid = rt_thread_create("touch",
                               touch_thread, RT_NULL,
                               TOUCH_STK_SIZE,
                               TOUCH_TASK_PRIO, 10);

    if (btn_tid != RT_NULL)![image-20210530164629809](C:\Users\zbr\AppData\Roaming\Typora\typora-user-images\image-20210530164629809.png)
        rt_thread_startup(btn_tid);

    return RT_EOK;
}
INIT_APP_EXPORT(Gui_touch_init);
```

### **4.5、编译、下载，验证**



![](https://img-blog.csdnimg.cn/20210530170416151.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



让我们点击一下中间的两个按钮，观察串口输出：

![](https://img-blog.csdnimg.cn/20210530170427918.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

至此，我们在RT-Thread上面移植STemwin已经成功的得以已验证。



---



# 玩转RT-Thread系列教程(7)--移植LittleVGL



## 一、LVGL介绍

> LittlevGL是一个免费的开放源代码图形库，它提供创建嵌入式GUI所需的一切，它具有易于使用的图形元素，精美的视觉效果和低内存占用。
>
> 没错，它是一个开源的图像库，旨在为嵌入式设备提供一个精美的界面，当然嵌入式设备只是一部分，由于它是使用标C所写，你可以很轻松的移植到更多设备上，包括但不局限于嵌入式设备。

说了这么多，大家心里肯定想，无图无真相！那么下面我们来看一下littlvGL的几个Demo，相信一定能让你惊叹：

<iframe height=480 width=600 src="https://www.waveshare.net/study/data/attachment/portal/202003/28/183152ybses0ove6vw0ocv.gif">


## 二、LVGL在RTT的移植

### 2.1、准备的文件

移植LVGL肯定少不了官方的核心文件，首先，让我们去官网下载，当然在这里我已经为大家准备好了，直接用网盘下载即可

链接：https://pan.baidu.com/s/1sluL_kZ8vuGTL78kkOryiA 提取码：f20y 

![](https://img-blog.csdnimg.cn/20210601094712340.png)

### 2.2、LVGL文件加入工程文件夹

![](https://img-blog.csdnimg.cn/20210601094850387.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



### 2.3、添加SConscript构建脚本

​	2.3.1、为什么添加SConscript？

​		SConscript是用python写的脚本，目的就是可以让我们使用env图形化去就生成自己定制的软件包。

​		否则如果我们每次在mdk中手动添加配置文件，会造成很多不必要的劳动力，所以稍稍介绍一下一劳永逸的配置方法。

​	2.3.2、怎么添加配置脚本？

​			我已经将配置方法详细的写了出来：

![](https://img-blog.csdnimg.cn/20210531215611436.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

然后我们找到board下的Kconfig文件，用Notepad++打开

![](https://img-blog.csdnimg.cn/20210601095531219.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601095948361.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



### 2.4、使用env选择软件包

![](https://img-blog.csdnimg.cn/20210601100107585.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601100117813.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601100127945.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601100134538.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

以上步骤就可以选择我们自己定制的软件包了，感兴趣的朋友可以参考一下Kconfig文件，配置选项是不是都是一一对应的呢

### 2.5、生成mdk工程

![](https://img-blog.csdnimg.cn/20210530165904107.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.6、验证是否添加成功

如果目录中和我的一样，那么我们已经成功完成了了90%

![](https://img-blog.csdnimg.cn/20210601100514420.png)

![](https://img-blog.csdnimg.cn/20210601100611944.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601100626855.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

然后我们添加一下，官方自带的示例代码：

![](https://img-blog.csdnimg.cn/20210601100751879.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.7、编译、下载、验证

![](https://img-blog.csdnimg.cn/20210601101752189.png)

![](https://img-blog.csdnimg.cn/20210601102026585.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

## 三、LVGL详细配置

既然我们已经成功的验证了LVGL，那么让我们深入的研究一下，lvgl是怎么调用显示以及触摸接口的

### 3.1、显示接口

![](https://img-blog.csdnimg.cn/2021060110270660.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601102904816.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 3.2、触摸接口

![](https://img-blog.csdnimg.cn/2021060110305631.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

---

# 玩转RT-Thread系列教程(8)--ADC—光敏数据的采集

## 一、查看电路原理图

![](https://img-blog.csdnimg.cn/20210601124548192.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![光敏电阻](https://img-blog.csdnimg.cn/20210601124652482.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

## 二、添加ADC—DMA配置

### 2.1、配置Cubemx

![](https://img-blog.csdnimg.cn/20210601130042771.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​	为ADC设置频率，72/6=12分频

![](https://img-blog.csdnimg.cn/20210601125810970.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![DMA配置](https://img-blog.csdnimg.cn/20210601125915392.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![关闭终端](https://img-blog.csdnimg.cn/20210601130138368.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

生成代码

### 2.2、添加Kconfig中ADC配置

![](https://img-blog.csdnimg.cn/20210601125035976.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.3、打开env，配置ADC3

menuconfig

![](https://img-blog.csdnimg.cn/20210601125235333.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601125240225.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601125253621.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601125302839.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.4、保存配置，生成mdk工程

![](https://img-blog.csdnimg.cn/20210601125347449.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601125404120.png)

## 三、ADC外设的使用

### 3.1、添加ADC外设初始化

![](https://img-blog.csdnimg.cn/20210601130421990.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

打开stm32f1xx_hal_msp.c我们可以看见，cubemx生成的代码自动添加到了该文件中。

接下来我们打开main.c复制一下初始化部分。

![](https://img-blog.csdnimg.cn/20210601130631950.png)

```c
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
  sConfig.Channel = ADC_CHANNEL_5;
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
```

### 3.2、获取光敏传感器数值

因为开发板IO引脚的电压最高为3.3v，同时ADC采用16字节对齐，所以最大数值为4096，经过换算得出换算电压公式：

> V(adc) = ADC * 3.3 / 4096

完整代码：

```c
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "button_app.h"

ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;

//定义key 控制块
struct key_state_type key0 = { 0 };
struct key_state_type key1 = { 0 };

static void MX_ADC3_Init(void);
static void MX_DMA_Init(void);
static void Get_Light_Value(void);

static uint16_t adc_data;
static uint32_t light_value;

int main(void)
{
		MX_DMA_Init();
		MX_ADC3_Init();
		HAL_ADC_Start(&hadc3);
		HAL_ADC_Start_DMA(&hadc3,(uint32_t*)&adc_data,(uint32_t)1);
	
    while (1)
    {
				Get_Light_Value();
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

void Get_Light_Value(void)
{
		light_value = adc_data * 330/4096;
		rt_kprintf("light value is:%d.%02d\r\n",light_value/100,light_value%100);
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
```

### 3.3、编译、下载、验证

![adc](https://img-blog.csdnimg.cn/20210601134430412.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

可以看见当用手去靠近光敏传感器时，电压明显增加。

## 四、结合LVGL使用

上次我们学习了LVGL的移植，那么我们今天就将其应用起来做一个综合案例吧

### 4.1、案例分析

使用LVGL创建仪表盘，通过仪表盘来动态显示我们采集到的ADC数据。

### 4.2、编码实现

**4.2.1、创建LVGL线程**

```c
static int _lv_init = 0;
static void lvgl_tick_run(void *p)
{
    if (_lv_init)
    {
      	//LVGL心跳
        lv_tick_inc(1);
    }
}

static int lvgl_tick_handler_init(void)
{
    rt_timer_t timer = RT_NULL;
    int ret;

    timer = rt_timer_create("lv_tick", lvgl_tick_run, RT_NULL, 1, RT_TIMER_FLAG_PERIODIC);

    if (timer == RT_NULL)
    {
        return RT_ERROR;
    }

    ret = rt_timer_start(timer);
    return ret;
}

static void lvgl_th_run(void *p)
{
		tp_dev.init();						 //触摸屏初始化
		//
    lv_init();								 //lvgl系统初始化
    lv_port_disp_init();				//lvgl显示接口初始化
    lv_port_indev_init();				//lvgl输入接口初始化	
		_lv_init = 1;							 //开启心跳
    lvgl_tick_handler_init();		//心跳定时器
		//
		lv_test_theme_1(lv_theme_night_init(210,NULL));
		while(1)
    {
			  tp_dev.scan(0);
        lv_task_handler();
        rt_thread_mdelay(5);
    }
}

int rt_lvgl_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_thread_t thread = RT_NULL;

    thread = rt_thread_create("lvgl", lvgl_th_run, RT_NULL, 2048, 15, 10);

    if(thread == RT_NULL)
    {
        return RT_ERROR;
    }

    rt_thread_startup(thread);

    return RT_EOK;
}
INIT_APP_EXPORT(rt_lvgl_init);
```

**4.2.2、编写UI显示**

```c
/*********************
 *      INCLUDES
 *********************/
#include "lv_test_theme_1.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <stdio.h>

#if LV_USE_TESTS
/*********************
 *      DEFINES
 *********************/
/* defined the LED0 pin: PB5 */
#define LED0_PIN    GET_PIN(B, 5)
/* defined the LED1 pin: PE5 */
#define LED1_PIN    GET_PIN(E, 5)
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_tab1(lv_obj_t * parent);
static void create_tab2(lv_obj_t * parent);
static void create_tab3(lv_obj_t * parent);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_style_t gauge_style;
lv_obj_t * gauge1;

lv_obj_t * label1;

//结构体赋值
lv_color_t needle_colors1[1];//指针的颜色

extern float light_value;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void lv_test_theme_1(lv_theme_t * th)
{
    lv_theme_set_current(th);
    th = lv_theme_get_current();
    lv_obj_t * scr = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(scr);

    lv_obj_t * tv = lv_tabview_create(scr, NULL);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_t * tab1 = lv_tabview_add_tab(tv, "RB_Team");

    lv_gauge_test_start(tab1);
}

//任务回调函数
void task_cb(lv_task_t * task)
{
    //设置指针的数值
    lv_gauge_set_value(gauge1, 0, (rt_int16_t)light_value * 10);
}

void lv_gauge_test_start(lv_obj_t * parent)
{
    //1.创建自定义样式
    lv_style_copy(&gauge_style, &lv_style_pretty_color);
    gauge_style.body.main_color = LV_COLOR_MAKE(0x5F, 0xB8, 0x78);  //关键数值点之前的刻度线的起始颜色,为浅绿色
    gauge_style.body.grad_color =  LV_COLOR_MAKE(0xFF, 0xB8, 0x00); //关键数值点之前的刻度线的终止颜色,为浅黄色
    gauge_style.body.padding.left = 13;//每一条刻度线的长度
    gauge_style.body.padding.inner = 8;//数值标签与刻度线之间的距离
    gauge_style.body.border.color = LV_COLOR_MAKE(0x33, 0x33, 0x33); //中心圆点的颜色
    gauge_style.line.width = 4;//刻度线的宽度
    gauge_style.text.color = LV_COLOR_WHITE;//数值标签的文本颜色
    gauge_style.line.color = LV_COLOR_OLIVE;//关键数值点之后的刻度线的颜色

    //仪表盘
    gauge1 = lv_gauge_create(parent, NULL);//创建仪表盘
    lv_obj_set_size(gauge1, 300, 300); //设置仪表盘的大小
    lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &gauge_style); //设置样式
    lv_gauge_set_range(gauge1, 0, 50); //设置仪表盘的范围
    needle_colors1[0] = LV_COLOR_TEAL;
    lv_gauge_set_needle_count(gauge1, 1, needle_colors1); //设置指针的数量和其颜色
    lv_gauge_set_value(gauge1, 0, (rt_int16_t)light_value * 10); //设置指针1指向的数值
    lv_gauge_set_critical_value(gauge1, 40); //设置关键数值点
    lv_gauge_set_scale(gauge1, 240, 41, 10); //设置角度,刻度线的数量,数值标签的数量
    lv_obj_align(gauge1, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0); //设置与屏幕居中对齐

    //3.创建一个标签来显示指针1的数值
    label1 = lv_label_create(parent, NULL);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK); //设置长文本模式
    lv_obj_set_width(label1, 80); //设置固定的宽度
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER); //设置文本居中对齐
    lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &lv_style_pretty); //设置样式
    lv_label_set_body_draw(label1, true); //使能背景重绘制
    lv_obj_align(label1, gauge1, LV_ALIGN_CENTER, 0, 60); //设置与gauge1的对齐方式
    lv_label_set_text(label1, "0.0 V/h"); //设置文本
    lv_label_set_recolor(label1, true); //使能文本重绘色

    //4.创建一个任务来模拟速度指针的变化
    lv_task_create(task_cb, 300, LV_TASK_PRIO_MID, NULL);
}

#endif /*LV_USE_TESTS*/
```

### **4.3、编译、下载、验证**

![](https://img-blog.csdnimg.cn/2021060115350545.GIF#pic_center)

---

# 玩转RT-Thread系列教程(9)--485总线读取温湿度

## 一、485总线介绍

485（一般称作 RS485/EIA-485）是隶属于 OSI 模型物理层的电气特性规定为 2 线，半双工，多点通信的标准。它的电气特性和 RS-232 大不一样。用缆线两端的电压差值来表示传递信号。RS485 仅仅规定了接受端和发送端的电气特性。它没有规定或推荐任何数据协议。

RS485 的特点包括：

> 1） 接口电平低，不易损坏芯片。RS485 的电气特性：逻辑“1”以两线间的电压差为+(2~6)V表示；逻辑“0”以两线间的电压差为-(2~6)V 表示。接口信号电平比 RS232 降低了，不易损坏接口电路的芯片，且该电平与 TTL 电平兼容，可方便与 TTL 电路连接。 
>
> 2） 传输速率高。10 米时，RS485 的数据最高传输速率可达 35Mbps，在 1200m 时，传输速度可达 100Kbps。 
>
> 3） 抗干扰能力强。RS485 接口是采用平衡驱动器和差分接收器的组合，抗共模干扰能力增强，即抗噪声干扰性好。
>
> 4） 传输距离远，支持节点多。RS485 总线最长可以传输 1200m 以上（速率≤100Kbps）一般最大支持 32 个节点，如果使用特制的 485 芯片，可以达到 128 个或者 256 个节点，最大的可以支持到 400 个节点。

![在这里插入图片描述](https://img-blog.csdnimg.cn/20210601185404165.png)

## 二、485温湿度传感器使用

### **2.1、使用PC测试485通信**

首先，先按照规定接线方式，进行安装连接，连接好后，打开CommMontor串口监控精灵，如下图：

链接：https://pan.baidu.com/s/1Q6jRuoZfVMEH-rt7gZ5BNw  提取码：y89c 

![](https://img-blog.csdnimg.cn/2021060119512339.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210601195856164.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 其中	01 03 00 00 00 02 C4 0B为问询帧，所以只要串口发送这一串十六进制即可得到485总线回复

### **2.2、使用RT-Thread进行485通信**

根据我们之前串口实验，我们只需稍作修改即可完成485的通信。

**2.2.1、编写485通信代码**

```c
uint16_t temp = 0;  //定义温度变量
uint16_t humi = 0;  //定义湿度变量

char tem[10] = {0}; //存放温度数据的数组
char hum[10] = {0}; //存放湿度数据的数组

unsigned char sensor_T_H[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B}; //温湿度传感器

static void serial_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));

        rt_device_write(serial, 0, sensor_T_H, sizeof(sensor_T_H));  //发送采集数据指令

        /* 从消息队列中读取消息*/
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);

        if (result == RT_EOK)
        {
            /* 从串口读取数据*/
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);

            for(uint8_t a = 0; a < 8; a++)
            {
                rt_kprintf("sensor[%d] = %.2x \r\n", a, rx_buffer[a]);
            }

            temp = 1.0 * ((rx_buffer[3] << 8) + rx_buffer[4]);
            humi = 1.0 * ((rx_buffer[5] << 8) + rx_buffer[6]);

            sprintf((char*)tem, "%.2f", (float)temp / 100); //拼接到温度数组里
            sprintf((char*)hum, "%.2f", (float)humi / 100); //拼接到湿度数组里

            /* 打印数据 */
            rt_kprintf("tem:[%s]	hum:[%s]\r\n", tem, hum);
						
						rt_thread_mdelay(1000);
        }
    }
}
```

当然了，不要忘记，485问询串口的比特率是9600！！所以我们需要在串口初始化添加修改波特率的配置。

```c
    config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.bufsz     = 64;                   	//修改缓冲区 buff size 为 128
    config.parity    = PARITY_NONE;           //无奇偶校验位
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
```

![](https://img-blog.csdnimg.cn/20210601200859815.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### **2.3、编译、下载、查看结果**

![](https://img-blog.csdnimg.cn/20210601201058730.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 通过串口可以清晰的看到485设备返回的帧，通过拼接即可得到正确的温度湿度数据。

## 三、使用LVGL显示温湿度数据

> 这次我们要结合LVGL来动态显示温湿度数据。为了达到直观的效果，这里的显示我用折线图来显示，用到的控件为lv_chart。

### **3.1、编码设计**

```c
extern int16_t temp;  //定义温度变量
extern int16_t humi;  //定义湿度变量

const lv_coord_t series1_y[POINT_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const lv_coord_t series2_y[POINT_COUNT] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

lv_style_t chart_style;
lv_chart_series_t * series1;
lv_chart_series_t * series2;

//任务回调函数
void task_cb(lv_task_t * task)
{
		//往series1数据线上添加新的数据点
		lv_chart_set_next(chart1,series1,temp);
		//往series2数据线上添加新的数据点
		lv_chart_set_next(chart1,series2,humi);
}

void lv_gauge_test_start(lv_obj_t * parent)
{
		//1.创建样式
		lv_style_copy(&chart_style,&lv_style_pretty);
		chart_style.body.main_color = LV_COLOR_WHITE;
		chart_style.body.grad_color = chart_style.body.main_color;
		chart_style.body.border.color = LV_COLOR_BLACK;
		chart_style.body.border.width = 3;
		chart_style.body.border.opa = LV_OPA_COVER;
		chart_style.body.radius = 1;
		chart_style.line.color = LV_COLOR_GRAY;
		chart_style.text.color = LV_COLOR_WHITE;

		//2.创建图表对象
		chart1 = lv_chart_create(parent,NULL);
		lv_obj_set_size(chart1,250,200);
		lv_obj_align(chart1,NULL,LV_ALIGN_IN_RIGHT_MID,-70,0);
		lv_chart_set_type(chart1,LV_CHART_TYPE_LINE);
		lv_chart_set_series_opa(chart1,LV_OPA_80);
		lv_chart_set_series_width(chart1,4);
		lv_chart_set_series_darking(chart1,LV_OPA_80);
		lv_chart_set_style(chart1,LV_CHART_STYLE_MAIN,&chart_style);
		lv_chart_set_point_count(chart1,POINT_COUNT);
		lv_chart_set_div_line_count(chart1,4,4);
		lv_chart_set_range(chart1,0,100);
		lv_chart_set_y_tick_length(chart1,10,3);
		lv_chart_set_y_tick_texts(chart1,"100\n90\n80\n70\n60\n50\n40\n30\n20\n10\n0",5,LV_CHART_AXIS_DRAW_LAST_TICK);
		lv_chart_set_x_tick_length(chart1,10,3);
		lv_chart_set_x_tick_texts(chart1,"0\n2\n4\n6\n8\n10",5,LV_CHART_AXIS_DRAW_LAST_TICK);
		lv_chart_set_margin(chart1,40);
		//2.2 往图表中添加第1条数据线
		series1 = lv_chart_add_series(chart1,LV_COLOR_RED);
		lv_chart_set_points(chart1,series1,(lv_coord_t*)series1_y);
		
		//2.3 往图表中添加第2条数据线
		series2 = lv_chart_add_series(chart1,LV_COLOR_BLUE);
		lv_chart_set_points(chart1,series2,(lv_coord_t*)series2_y);

		lv_chart_refresh(chart1);
		
    //4.创建一个任务来显示变化
    lv_task_create(task_cb, 300, LV_TASK_PRIO_MID, NULL);
}
```

### **3.2、实现效果**

![](https://img-blog.csdnimg.cn/20210602104608816.GIF#pic_center)

当握住温湿度传感器我们可以直观的看到，温度和湿度发生了明显变化，是不是很有成就感呢。

---



# 玩转RT-Thread系列教程(10)--文件系统使用

## 一、文件系统介绍

RT-Thread的文件系统是一套实现了数据的存储、分级组织、访问和获取等操作的抽象数据类型 ，是一种用于向用户提供底层数据访问的机制。

RT-Thread DFS 组件的主要功能特点有：

- 为应用程序提供统一的 POSIX 文件和目录操作接口：read、write、poll/select 等。
- 支持多种类型的文件系统，如 FatFS、RomFS、DevFS 等，并提供普通文件、设备文件、网络文件描述符的管理。
- 支持多种类型的存储设备，如 SD Card、SPI Flash、Nand Flash 等。

![RT-Thread官方文档](https://img-blog.csdnimg.cn/20210602121209439.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

更多关于RTT文件系统可以查看RTT[官方文档](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/filesystem/filesystem)

## 二、文件系统的配置

### **2.1、打开cubemx配置**

![在这里插入图片描述](https://img-blog.csdnimg.cn/2021060211265813.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### **2.2、env使能SD设备+文件系统**

menuconfig

![](https://img-blog.csdnimg.cn/20210602112530362.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210602112517559.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### **2.3、生成mdk工程**

![](https://img-blog.csdnimg.cn/20210602115802947.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

## 三、文件系统使用

### **3.1、下载验证**

![](https://img-blog.csdnimg.cn/20210602113042889.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 可以看到，RTT组件已经为我们自动的识别出了SD设备

### **3.2、编写挂载代码**

**3.2.1、添加FileSyetem.c文件**

![](https://img-blog.csdnimg.cn/20210602113623273.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

**3.2.2、在FileSyetem编写业务逻辑**

![](https://img-blog.csdnimg.cn/20210602115342415.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/2021060211544823.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

具体代码：

```c
#include "FileSystem.h"

//SD卡挂完成的信号量
static rt_sem_t SD_CardFinsh_mutex = RT_NULL;
static rt_thread_t thread_filesystem;

static void MountSDcard(void)
{
    rt_device_t dev;

    while (1)
    {
        dev = rt_device_find("sd0");

        if (dev != RT_NULL)
        {
            if (dfs_mount("sd0", "/", "elm", 0, 0) == RT_EOK)
            {
                rt_kprintf("SD mount to / success\n");
                //释放SD_CardFinsh_mutex信号量
                rt_sem_release(SD_CardFinsh_mutex);
            }
            else
            {
                rt_kprintf("SD mount to / failed\n");
            }
            break;
        }
        rt_thread_delay(50);
    }
}

void FlieSystem_entry(void *parameter)
{
    static rt_err_t result;

    MountSDcard();

    while (1)
    {
        /* 永久方式等待信号量*/
        result = rt_sem_take(SD_CardFinsh_mutex, RT_WAITING_FOREVER);

        if (result != RT_EOK)
        {
            rt_kprintf("t2 take a SD_CardFinsh_mutex semaphore, failed.\n");
            rt_sem_delete(SD_CardFinsh_mutex);
            return;
        }
        else
        {
            rt_sem_delete(SD_CardFinsh_mutex);
            return;
        }
    }
}

static int FileSystemInit(void)
{
    /* 创建一个信号量 */
    SD_CardFinsh_mutex = rt_sem_create("SDCard_mutex", 0, RT_IPC_FLAG_FIFO);

    //创建sd线程
    thread_filesystem = rt_thread_create("file_sys", FlieSystem_entry, RT_NULL, 2048, 20, 10);

    if (thread_filesystem != RT_NULL)
    {
        rt_thread_startup(thread_filesystem);
    }
}

INIT_APP_EXPORT(FileSystemInit);
```

### **3.3、编译、下载、验证**

| **FinSH 命令** | **描述**                                                     |
| -------------- | ------------------------------------------------------------ |
| ls             | 显示文件和目录的信息                                         |
| cd             | 进入指定目录                                                 |
| cp             | 复制文件                                                     |
| rm             | 删除文件或目录                                               |
| mv             | 将文件移动位置或改名                                         |
| echo           | 将指定内容写入指定文件，当文件存在时，就写入该文件，当文件不存在时就新创建一个文件并写入 |
| cat            | 展示文件的内容                                               |
| pwd            | 打印出当前目录地址                                           |
| mkdir          | 创建文件夹                                                   |
| mkfs           | 格式化文件系统                                               |

![](https://img-blog.csdnimg.cn/2021060211511914.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210602122025467.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 可以看见SD卡挂载成功，下一次我们要利用RTT的文件系统存储一些数据等等

---



# 玩转RT-Thread系列教程(11)--综合485通信+文件系统

> 根据前两篇文章我们学习了485总线读取温湿度数据+虚拟文件系统的使用，今天让我们来结合二者进行一次综合实战

## 一、案例分析

* 1.挂载SD卡
* 2.获取温湿度数据
* 3.在创建SD卡中创建文件，保存数据
* 4.将异常温湿度数据保存到SD文件中

## 二、系统优化

> 在进行我们今天的综合案例前，我们先对我们之前的代码进行优化

### 2.1、SD热插处理

```c
void FlieSystem_entry(void *parameter)
{
    static rt_err_t result;

    rt_device_t dev;

    while(1)
    {
        dev = rt_device_find("sd0");

        if (dev != RT_NULL)
        {
            if (dfs_mount("sd0", "/", "elm", 0, 0) == RT_EOK)
            {
                rt_kprintf("SD mount to / success\n");
                break;
            }
            else
            {
                rt_kprintf("SD mount to / failed\n");
            }
        }
        rt_thread_mdelay(500);
    }
}

static int FileSystemInit(void)
{
    //创建sd线程
    rt_thread_t thread_filesystem = rt_thread_create("file_sys", FlieSystem_entry, RT_NULL, 1024, 18, 20);

    if (thread_filesystem != RT_NULL)
    {
        rt_thread_startup(thread_filesystem);
    }
}

INIT_ENV_EXPORT(FileSystemInit);
```

### 2.2、温湿度数据发送

之前我们读取到的温湿度数据都是直接打印出来或者通过全局变量进行处理，这对于简单的工程我们是可以接受的，但对于复杂的功能来说，我们需要进行线程之间的通信来获取其他线程的数据。这里就用到了我们之前学习到的**消息邮箱**机制。

发送线程：

![](https://img-blog.csdnimg.cn/20210604183034694.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210604183148584.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

接收线程：

![](https://img-blog.csdnimg.cn/20210604183240318.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

通过永久等待方式等待邮箱消息的来到。

### 2.3、创建ADC处理线程

![](https://img-blog.csdnimg.cn/20210604183407686.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210604183424801.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

## 三、案例实战

### 3.1、文件的读写操作

这里我操作文件使用的是POSIX方式

```c
//保存数据到SD
void Sensor_DataTo_SD(char* buff)
{
    /* 以创建和读写模式打开 /text.txt 文件，如果该文件不存在则创建该文件 */
    FILE *recvdata_p0;
    recvdata_p0 = fopen("/Sensor_Data.csv", "a+");
    if (recvdata_p0 != RT_NULL)
    {
        fputs(buff, recvdata_p0);
        fputs("\n", recvdata_p0);
        fclose(recvdata_p0);
    }
}

//从SD读取信息
void Data_ReadFSD(void)
{
    FILE *fp;
    char buffer[120];
    fp = fopen("/Sensor_Data.csv", "r");
    if (fp != RT_NULL)
    {
        fread(buffer, sizeof(char), sizeof(buffer), fp);
        rt_kprintf("%s", buffer);
        fclose(fp);
    }
}
```

### 3.2、温度阈值判断逻辑

> 判断阈值逻辑我是这样写的，当超过最高/低设定阈值时，通过累加器去进行累加，当超过一定次数便真正进行警报处理

```c
void Save_Data_TOSD(float data1, float data2)
{
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
				rt_kprintf("温度超过标准！%d\n",detect_logic.T_Count_Alarm);
				sprintf((char*)detect_logic.Alarm_buff, "Temp:%.2f,humi:%.2f", data1, data2); //拼接到温度数组里
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
				rt_kprintf("湿度超过标准！%d\n",detect_logic.H_Count_Alarm);
				sprintf((char*)detect_logic.Alarm_buff, "Temp:%.2f,humi:%.2f", data1, data2); //拼接到温度数组里
				//保存异常数据到SD卡
				Sensor_DataTo_SD((char*)detect_logic.Alarm_buff);
		}
}
```

### 3.3、利用事件进行线程同步

既然利用到了事件，那么我们先来介绍一下RTT的事件集的作用以及用法：

**3.3.1、事件的介绍**

> 事件集是线程间同步的机制之一，一个事件集可以包含多个事件，利用事件集可以完成一对多，多对多的线程间同步。

事件集主要用于线程间的同步，与信号量不同，它的特点是可以实现一对多，多对多的同步。即一个线程与多个事件的关系可设置为：其中任意一个事件唤醒线程，或几个事件都到达后才唤醒线程进行后续的处理；同样，事件也可以是多个线程同步多个事件。

RT-Thread 定义的事件集有以下特点：

* 事件只与线程相关，事件间相互独立：每个线程可拥有 32 个事件标志，采用一个 32 bit 无符号整型数进行记录，每一个 bit 代表一个事件；

* 事件仅用于同步，不提供数据传输功能；

* 事件无排队性，即多次向线程发送同一事件 (如果线程还未来得及读走)，其效果等同于只发送一次。

在 RT-Thread 中，每个线程都拥有一个事件信息标记，它有三个属性，分别是 **RT_EVENT_FLAG_AND(逻辑与)**，**RT_EVENT_FLAG_OR(逻辑或）**以及 **RT_EVENT_FLAG_CLEAR(清除标记）**。当线程等待事件同步时，可以通过 32 个事件标志和这个事件信息标记来判断当前接收的事件是否满足同步条件。

**3.3.2、事件在系统中的使用**

因为我们分别有ADC线程，温湿度获取线程等多个线程，假设一个线程采集数据很快，另一个线程采集速度很慢，那么假设我们不使用线程间同步方式进行处理的话，很容易会导致数据的不一致性。所以我们使用事件方式来处理线程之间的同步。

![](https://img-blog.csdnimg.cn/20210604210256380.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210604210327143.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 3.4、接收线程处理

![](https://img-blog.csdnimg.cn/20210604210410713.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 在接收线程中，这里我参考了[WillianChan](https://github.com/willianchanlovegithub)在分布式温度监控系统中对接收数据线程的方法，使用一个永久等待接收的事件以及超时接收事件来优化文件存储耗时的问题——只有当超过阈值时才进行数据的转存。

```c
static void Publish_th(void *pram)
{
    //传感器数据结构体
    Sensor_msg sensor_msg;
    static rt_uint32_t e;
    static char r_buff[64];

    while(1)
    {
        if (rt_event_recv(Sensor_event, EVENT_ADC_FLAG, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e) != RT_EOK)
            continue;

        //5s等待接收邮件
        if(rt_mb_recv(Sensor_msg_mb, (rt_ubase_t*)&sensor_msg, rt_tick_from_millisecond(5000)) == RT_EOK)
        {
          sprintf((char*)r_buff, "Temp:%.2f,humi:%.2f", (float)sensor_msg->temp, (float)sensor_msg->hum); //拼接到温度数组里
          rt_kputs((char*)r_buff);
          rt_kputs("\n");

          //阈值判断逻辑
          Save_Data_TOSD((float)sensor_msg->temp, (float)sensor_msg->hum);

          //释放内存块
          rt_mp_free(sensor_msg);
          sensor_msg = RT_NULL;
          continue;            
        }
				//5s超时直接存数据
				//阈值判断逻辑
				rt_kputs("@1s接收事件超时--存储数据\n");
				Sensor_DataTo_SD((char*)r_buff);
      	continue;
    }
}
```

### 3.5、编译、下载、验证

![](https://img-blog.csdnimg.cn/20210604211143538.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210604203642729.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 可以看到，当湿度超过阈值时，异常数据才会被保存到excell文档中。

---

# 玩转RT-Thread系列教程(12)--WIFI模组的使用

> 本章节也是最有意思的章节，我将带你手把手物联网，让你直观的感受RTT软件包的便捷以及强大之处！

## 一、添加at_device软件包

### 1.1、配置Cubemx

> 根据正点原子开发板原理图，我们可以看出wifi使用到了串口3，记得用跳线帽将串口3与GBC接口连接。

![](https://img-blog.csdnimg.cn/20210605123636796.png)

![](https://img-blog.csdnimg.cn/20210605123643357.png)

> 使能串口3

![ ](https://img-blog.csdnimg.cn/20210605094801172.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 1.2、打开env配置

> 使能串口3

![](https://img-blog.csdnimg.cn/20210605093746562.png)

![](https://img-blog.csdnimg.cn/20210605093739211.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 1.3、使能at_device

![](https://img-blog.csdnimg.cn/20210605093618896.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210605093611156.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210605093604479.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210605093553532.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 配置用户信息，输入您的wifi和密码

![](https://img-blog.csdnimg.cn/20210605093537741.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 1.4、更新软件包

![](https://img-blog.csdnimg.cn/2021060509472414.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 1.5、生成mdk工程

![](https://img-blog.csdnimg.cn/20210605094745854.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

## 二、启动测试AT组件

> 打开已经生成好的mdk工程，我们发现自动添加了两个文件，分别为AT、at_device

![](https://img-blog.csdnimg.cn/20210605124716861.png)

### 2.1、测试前的准备工作

**2.1.1、首先让我们先来了解一下AT指令吧**

**AT 命令集是一种应用于 AT 服务器（AT Server）与 AT 客户端（AT Client）间的设备连接与数据通信的方式**。

![](https://img-blog.csdnimg.cn/20210605125515975.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

* AT 功能的实现需要 AT Server 和 AT Client 两个部分共同完成。
* 前缀由字符 AT 构成，主体由命令、参数和可能用到的数据组成；结束符一般为 `<CR><LF>` (`"\r\n"`)。
* AT Server 和 AT Client 之间支持多种数据通讯的方式（UART、SPI 等），最常用的是串口 UART 通讯方式。

> 本次我们使用的通信模组为安信可公司出版的ESP8266，以下是改通讯模组常用AT指令：

* AT+RST	             复位模组
* AT+GMR                查询版本信息
* AT+CWMODE=1    开启模组透传
* AT+CWJAP="WIFI名称","WIFI密码"   模组作为STA模式连接wifi
* AT+CIFSR              查看分配的IP地址
* AT+CIPSTART="TCP","xxxxx",1883    使用TCP方式连接服务器
* AT+CIPMODE=1    开启透传
* AT+CIPSEND         发送数据

![](https://img-blog.csdnimg.cn/20210605133230832.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.2、修改at_client+at_device_esp8266文件

> 修改响应最大支持的接收数据的长度为128，防止缓冲区内存过小

![](https://img-blog.csdnimg.cn/20210605130521777.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 将rt_thread_mdelay函数的延迟时间修改成1000ms，让AT+RST有足够时间来运行

![](https://img-blog.csdnimg.cn/2021060512523699.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.3、编译、下载、验证

![](https://img-blog.csdnimg.cn/20210605132018700.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 可以看到，AT_Client已经成功初始化，并且模组已经成功的连接到wifi。

测试联网

![](https://img-blog.csdnimg.cn/2021060513205761.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 可以看到输入ifconfig已经成功的获取到了ip地址

![](https://img-blog.csdnimg.cn/20210605132333885.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> Ping下百度,可以收到数据包,证明我们的wifi已经成功联网了



## 三、可能出现的错误汇总

### 3.1、超时问题

```c
 \ | /
- RT -     Thread Operating System
 / | \     4.0.2 build Aug 16 2019
 2006 - 2019 Copyright by rt-thread team
[32m[I/sal.skt] Socket Abstraction Layer initialize success.[0m
[32m[I/at.clnt] AT client(V1.3.0) on device uart3 initialize success.[0m
[0m[D/at.dev] the network interface device(esp0) set up status[0m
[0m[D/at.dev] esp8266 device(esp0) initialize start.[0m
[31m[E/at.clnt] Read response buffer failed. The Response buffer size is out of buffer size(256)![0m
[0m[D/at.clnt] execute command (ATE0) timeout (5000 ticks)![0m
[32m[I/at.dev] esp8266 device(esp0) initialize retry...[0m
```

1.Read response buffer failed. 缓冲区内存过小，那我们就直接增大就好了

2.command (ATE0)运行超时，ATE0是AT指令中的关闭回显，出现这个问题的原因可能是AT组件初始化时间过短

**解决缓冲区内存过小问题**

> 修改响应最大支持的接收数据的长度为128，防止缓冲区内存过小

![](https://img-blog.csdnimg.cn/20210605130521777.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

**解决ATE0运行超时问题**

```c
AT_SEND_CMD(client, resp, "AT+RST");
/* reset waiting delay */
rt_thread_mdelay(2000);//增加延时
/* disable echo */
AT_SEND_CMD(client, resp, "ATE0");
```

### 3.2、不支持AT+CIPDNS_CUR?指令

![](https://img-blog.csdnimg.cn/20210605132858143.png)

解决`AT+CIPDNS_CUR?指令问题—原因：8266的固件版本太低

访问乐鑫的官网去下载更新版本的AT固件，https://www.espressif.com/zh-hans/support/download/at，并烧录固件即可

![](https://img-blog.csdnimg.cn/20210605133031655.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)



---

# 玩转RT-Thread系列教程(13)--MQTT协议通信

## 一、了解一下MQTT

### 1.1、MQTT介绍

![](https://img-blog.csdnimg.cn/2021060715344378.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

​	**客户端 Client**

​	使用MQTT的程序或设备。客户端总是通过网络连接到服务端。它可以

- 发布应用消息给其它相关的客户端。

- 订阅以请求接受相关的应用消息。

- 取消订阅以移除接受应用消息的请求。

- 从服务端断开连接。

  

  **服务端 Server**
  一个程序或设备，作为发送消息的客户端和请求订阅的客户端之间的中介。服务端

- 接受来自客户端的网络连接。

- 接受客户端发布的应用消息。

- 处理客户端的订阅和取消订阅请求。

- 转发应用消息给符合条件的已订阅客户端。

> 说白了一方为供应商，一方为消费者（Client），供应商（Server）一旦和消费者产生了联系，那么供应商（Server）就会提供商品给消费者，同时消费者（Client）也可以向供应商提供意见。

**订阅 Subscription**
		订阅包含一个主题过滤器（Topic Filter）和一个最大的服务质量（QoS）等级。订阅与单个会话（Session）关联。会话可以包含多于一个的订阅。会话的每个订阅都有一个不同的主题过滤器。

- QoS0，At most once，至多一次；Sender 发送的一条消息，Receiver 最多能收到一次，如果发送失败，也就算了。
- QoS1，At least once，至少一次；Sender 发送的一条消息，Receiver 至少能收到一次，如果发送失败，会继续重试，直到 Receiver 收到消息为止，但Receiver 有可能会收到重复的消息
- QoS2，Exactly once，确保只有一次。Sender 尽力向 Receiver 发送消息，如果发送失败，会继续重试，直到 Receiver 收到消息为止，同时保证 Receiver 不会因为消息重传而收到重复的消息。

### 1.2、MQTT协议数据包结构

> 一个MQTT数据包由：固定头（Fixed header）、可变头（Variable header）、有效载荷（payload）三部分构成。

- （1）固定头（Fixed header）。存在于所有MQTT数据包中，表示数据包类型及数据包的分组类标识。
- （2）可变头（Variable header）。存在于部分MQTT数据包中，数据包类型决定了可变头是否存在及其具体内容。
- （3）有效载荷（Payload）。存在于部分MQTT数据包中，表示客户端收到的具体内容。



| 固定报头 |
| -------- |

| byte1 |          | MQTT报文类型（1） | Reserved 保留位 |      |      |      |      |      |      |
| ----- | -------- | ----------------- | --------------- | ---- | ---- | ---- | ---- | ---- | ---- |
|       | 10       | 0                 | 0               | 0    | 1    | 0    | 0    | 0    | 0    |
| byte2 | 剩余长度 |                   |                 |      |      |      |      |      |      |
|       | 27       | 0                 | 0               | 1    | 0    | 0    | 1    | 1    | 1    |

| 可变报头 |
| -------- |

|        | **说明**     | **7** | **6** | **5** | **4** | **3** | **2** | **1** | **0** |
| ------ | ------------ | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| 协议名 |              |       |       |       |       |       |       |       |       |
| byte 1 | 长度 MSB (0) | 0     | 0     | 0     | 0     | 0     | 0     | 0     | 0     |
| byte 2 | 长度 LSB (4) | 0     | 0     | 0     | 0     | 0     | 1     | 0     | 0     |
| byte 3 | ‘M’          | 0     | 1     | 0     | 0     | 1     | 1     | 0     | 1     |
| byte 4 | ‘Q’          | 0     | 1     | 0     | 1     | 0     | 0     | 0     | 1     |
| byte 5 | ‘T’          | 0     | 1     | 0     | 1     | 0     | 1     | 0     | 0     |
| byte 6 | ‘T’          | 0     | 1     | 0     | 1     | 0     | 1     | 0     | 0     |

![](https://img-blog.csdnimg.cn/20210607160815218.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 这里我用Onenet的连接报文示例，其他的同理



## 二、RTT中MQTT组件

### 2.1、进入env，选择IOT组件

![](https://img-blog.csdnimg.cn/20210607172736633.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 2.2、选择Paho以及CJSON组件

![](https://img-blog.csdnimg.cn/20210607172832890.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> Eclipse paho是eclipse基金会下面的一个开源项目，基于MQTT协议的客户端
>
> json是一种轻量级的数据交换格式，简洁和清晰的层次结构使得 JSON 成为理想的数据交换语言，易于人阅读和编写
>
> cjson组件则是可以对服务器端发来的json数据进行解析，对比于常规的使用C库解析字符串方式，cjson为我们封装好了解析方法，调用更加灵活方便。

### 	2.3、更新、下载软件包

### 	2.4、生成mdk工程

![](https://img-blog.csdnimg.cn/20210602115802947.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

### 	2.5、打开mdk工程

![](https://img-blog.csdnimg.cn/20210610201621947.png)

可以看见，我们选择的paho以及cjson组件已经添加到了我们的工程中



## 三、MQTT的使用

### 3.1、编写MQTT客户端

1.宏定义连接mqtt服务器需要的参数：

```c
#define MQTT_Uri    "tcp://xxx.xxx.xxx:1883"     		// MQTT服务器的地址和端口号
#define ClientId    "751061401"                     // ClientId需要唯一
#define UserName    "rb"                        		// 用户名
#define PassWord    "123456"                        // 用户名对应的密码
```

2.定义一个mqtt客户端结构体变量

```c
/* 定义一个MQTT客户端结构体 */
static MQTTClient client;
```

3.对MQTT进行配置

```c
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
```

4.为MQTT的消息缓存申请内存

```c
    /* 为mqtt申请内存 */
    client.buf_size = client.readbuf_size = 1024;
    client.buf = rt_calloc(1, client.buf_size);
    client.readbuf = rt_calloc(1, client.readbuf_size);

    if (!(client.buf && client.readbuf))
    {
        rt_kprintf("no memory for MQTT client buffer!\r\n");
        return;
    }
```

5.设置回调函数，以及订阅主题；设置默认的回调函数，是在如果有订阅的 Topic 没有设置回调函数时，则使用该默认回调函数

```c
    /* 设置回调函数  */
    client.connect_callback = mqtt_connect_callback;
    client.online_callback = mqtt_online_callback;
    client.offline_callback = mqtt_offline_callback;

    /* 订阅一个主题，并设置其回调函数 */
    client.messageHandlers[0].topicFilter = rt_strdup(ONENET_MQTT_SUBTOPIC);
    client.messageHandlers[0].callback = mqtt_sub_callback;
    client.messageHandlers[0].qos = QOS1;

    /* 设置默认的回调函数 */
    client.defaultMessageHandler = mqtt_sub_default_callback;
```

6.启动 mqtt client

```c
    /* 启动 mqtt client */
		paho_mqtt_start(&client);
```

7.实现各个回调函数

```c
/* 默认的订阅回调函数，如果有订阅的 Topic 没有设置回调函数，则使用该默认回调函数 */
static void mqtt_sub_default_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *) msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    rt_kprintf("%.*s\r\n", msg_data->message->payloadlen, (char *) msg_data->message->payload);
}

/* 连接成功回调函数 */
static void mqtt_connect_callback(MQTTClient *c)
{
    rt_kprintf("success connect to mqtt! \r\n");
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
```

8.设置收到订阅主题的消息时的回调函数

```c
static void mqtt_sub_callback(MQTTClient *c, MessageData *msg_data)
{
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
		
		led1==1?LED1_ON:LED1_OFF;
		led2==1?LED2_ON:LED2_OFF;	
			
		if (NULL != root)
		{
				cJSON_Delete(root);
				root = NULL;
		}		
}
```

### 3.2、MQTT消息推送

> 既然我们已经使设备连接上了服务器，那么如何将我们的数据发送到MQTT服务器呢？
>
> 没错，我们需要向服务器端订阅的topic发送消息，使得服务器可以收到我们的“悄悄话”

paho已经已经为我们封装好了消息推送的函数--paho_mqtt_publish(MQTTClient *client, enum QoS qos, const char *topic, const char *msg_str)

**3.2.1、对之前处理进程同步的优化**

对于之前我们处理两个线程之间消息同步，我们采用的是通过事件来进行消息的同步。但经过测试发现了问题，我们没有必要去进行两次的事件的接收，

对于第二次我们完全可以直接通过消息邮箱进行判断是否收到了消息。

![之前的代码](https://img-blog.csdnimg.cn/20210610205058448.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![优化之后](https://img-blog.csdnimg.cn/20210610205444165.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

当我们收到了ADC采集到的光敏数据以及485的温湿度数据，我们就可以将传感器的数据发送到我们的服务器了。

```c
//ADC数据接收事件
if (rt_event_recv(Sensor_event, EVENT_ADC_FLAG, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e) != RT_EOK)
  continue;

//5s等待接收邮件
if(rt_mb_recv(Sensor_msg_mb, (rt_ubase_t*)&sensor_msg, rt_tick_from_millisecond(5000)) == RT_EOK)
{
  sprintf((char*)r_buff, "{\"temperature\":%.2f,\"humidity\":%.2f}", (float)sensor_msg->temp, (float)sensor_msg->hum); //拼接到温度数组里

  if(connect_sta == RT_TRUE && client.isconnected)
  {
    if(paho_mqtt_publish(&client, QOS1, "topic_pub", (char *) r_buff) != RT_EOK)
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

//10s超时直接存数据
rt_kputs("@5s接收事件超时--存储数据\n");
//发送采集数据指令
rt_device_write(serial, 0, sensor_T_H, sizeof(sensor_T_H));  
continue;
```



### 3.3、MQTT消息订阅

> 既然我们已经成功的将数据推送到了服务器，那么我们的设备如何收到MQTT服务器端发来的消息呢？
>
> 没错，接下来我们需要订阅服务器端发布的topic，使得设备可以收到服务器的“悄悄话”

我们只需要实现一下订阅主题的回调函数

```c
static void mqtt_sub_callback(MQTTClient *c, MessageData *msg_data){}
```

**3.3.1、使用CJSON解析数据**

> 因为我们服务器发送的数据是json格式的，这时候cjson便派上了用场

---

常用的cjson函数：

**void cJSON_Delete(cJSON *c)**

> 删除 cJSON 指针，由于我们会频繁的使用cJSON，也就是会频繁的申请内存， 这就相当于向内存借空间。
>
> 如果有借不还，很快就会将内存用空，导致系统崩溃。

**char  *cJSON_Print(cJSON *item)**

> 它是将cJSON数据解析成JSON字符串，并会在堆中开辟一块char *的内存空间，存放JSON字符串。
>
> 函数成功后会返回一个char *指针，该指针指向位于堆中JSON字符串。

**cJSON *cJSON_Parse(const char *value)**

>将一个JSON数据包，按照cJSON结构体的结构序列化整个数据包，并在堆中开辟一块内存存储cJSON结构体
>
>返回值：成功返回一个指向内存块中的cJSON的指针，失败返回NULL

**cJSON *cJSON_GetObjectItem(cJSON *object,const char *string)**

> 获取JSON字符串字段值，成功返回一个指向cJSON类型的结构体指针，失败返回NULL

**3.3.2、MQTT消息订阅**

```c
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

led1==1?LED1_ON:LED1_OFF;
led2==1?LED2_ON:LED2_OFF;	

if (NULL != root)
{
  cJSON_Delete(root);
  root = NULL;
}		
```

> 使用**cJSON_GetObjectItem**获取JSON字符串字段值，解析服务器发送的json字符串{“ledX”:1}
>
> 这里调用函数就会将字符串{“ledX”:1}进行拆分：“ledX的数据为1”，那么我们获取到了ledX的数值之后，岂不是想干嘛干嘛~



## 四、编译、下载、验证

![](https://img-blog.csdnimg.cn/20210610212012549.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

> 以上为设备启动后的消息，可以看出，当我们没有成功联网时mqtt会一直进行重连，直到成功联网后，才会订阅到topic以及进行消息的转发处理

![](https://img-blog.csdnimg.cn/20210610212205372.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzNjEwNDMw,size_16,color_FFFFFF,t_70)

![](https://img-blog.csdnimg.cn/20210610212049699.png)

> 这里我使用emq测试mqtt的通信，可以看到我们订阅的**topic_pub**主题成功的收到了设备发送的消息
>
> 同时向**topic_sub**推送消息设备也可以正常接收，至此我们的MQTT通信成功的得以已验证。

