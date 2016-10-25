/******************************************************************
** 文件名:	Meter.h
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#ifndef __Meter_H__
#define __Meter_H__
#include "include.h"

typedef enum { sub_open  = 1,sub_close,sub_reado,sub_readi } subcmd_t;

typedef struct {
	u8     addr;
	u8     ctrl;
	u16   Io_addr;
	u16   cmd;
	u16   crc;
	u16   bak;
} slave_t;


typedef struct Meter_t{
	void *parent;
	int (*meter_open)(struct Meter_t*,u8 slave_addr, u8 ndo);
	int (*meter_close)(struct Meter_t*,u8 slave_addr, u8 ndo);
	int (*meter_readi)(struct Meter_t*,u8 slave_addr,u8 ndo);
	int (*meter_reado)(struct Meter_t*,u8 slave_addr,u8 ndo);
	int (*meter_init)(struct Meter_t*,void*);
} Meter_t;

extern Meter_t g_meter;

#endif