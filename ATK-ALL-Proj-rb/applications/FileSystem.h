/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-20     rbbb       the first version
 */
#ifndef  FILESYSTEM_H
#define  FILESYSTEM_H
#include <rtthread.h>
#include <dfs_posix.h>
#include <rtdbg.h>

#include "dfs_fs.h"

void Sensor_DataTo_SD(char* buff);
int Mkdir_Function(void);
void Data_ReadFSD(void);

#endif
