/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-20     rbbb       the first version
 */
#include "FileSystem.h"

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

//保存数据到SD
void Sensor_DataTo_SD(char* buff)
{
    /* 以创建和读写模式打开 /text.txt 文件，如果该文件不存在则创建该文件 */
    int fd;

    fd = open("Sensor.txt", O_RDWR | O_APPEND | O_CREAT, 0);

    if (fd >= 0)
    {
        write(fd, buff, strlen(buff));
        close(fd);
    }
    else
    {
        rt_kprintf("open file:%s failed!\n", buff);
    }
}

//从SD读取信息
void Data_ReadFSD(void)
{
    struct dfs_fd fd;
    uint32_t length;
    char buffer[60];


    if (dfs_file_open(&fd, "Sensor.txt", O_RDONLY) < 0)
    {
        rt_kprintf("Open %s failed\n", "Sensor.txt");
        return;
    }

    do
    {
        memset(buffer, 0, sizeof(buffer));
        length = dfs_file_read(&fd, buffer, sizeof(buffer) - 1);

        if (length > 0)
        {
            rt_kprintf("%s", buffer);
        }
    }
    while (length > 0);

    rt_kprintf("\n");

    dfs_file_close(&fd);
}


static int FileSystemInit(void)
{
    //创建sd线程
    rt_thread_t thread_filesystem = rt_thread_create("file_sys", FlieSystem_entry, RT_NULL, 1024, 15, 20);

    if (thread_filesystem != RT_NULL)
    {
        rt_thread_startup(thread_filesystem);
    }
}

INIT_ENV_EXPORT(FileSystemInit);
