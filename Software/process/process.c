/******************************************************************
** 文件名:	process.c
** Copyright (c) 2016-2020 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:
**
** 版　本:	V1.0
*******************************************************************/
#include "process.h"
#include "database.h"

#define Mktime(_tm,p_data,_time) do{\
	(_tm)->tm_year	=	100 + *Pdata++;\
	(_tm)->tm_mon	= *Pdata++  -  1;\
	(_tm)->tm_mday	= *Pdata++;\
	(_tm)->tm_hour	= *Pdata++;\
	(_tm)->tm_min  	= *Pdata++;\
	(_tm)->tm_sec   	= *Pdata++;\
	_time = mktime(_tm);\
}while(0)

int Reset2DoFunctions(void)
{
	return SUCCESS;
}

int time_tick(u8 *package)
{
	assert_param(package,FAIL);
	/* 开发板设置时间格式
	 * date 051114542016.59  月 日 时 分 年.秒
	 * 把下发的时间格式化成字符串
	 */
	s8 	set_time[32]={0};
	#ifdef Config_EC_6018
		sprintf(set_time,"date %04u%02u%02u%02u%02u.%02u",\
			package[0]+2000,package[1],package[2],package[4],package[5],package[6]);
	#else
		sprintf(set_time,"date %02u%02u%02u%02u%04u.%02u",\
			package[1],package[2],package[4],package[5],package[0]+2000,package[6]);
	#endif
	//debug(DEBUG_chktime,"Time tick cmd:%s\n",set_time);
	system(set_time);
	return SUCCESS;
}

int Query_time(u8 *buf,int bufsize)
{
	assert_param(buf,FAIL);

	time_t SystemTime;
	struct tm TimeRTC;
	struct tm TimeSystem;

	memset(buf,0,bufsize);
	memset(&TimeRTC,0,sizeof(struct tm));
	memset(&TimeSystem,0,sizeof(struct tm));
	time(&SystemTime);
	TimeSystem = *localtime(&SystemTime);
	int  fd = open("/dev/rtc0",O_RDONLY,0);
	if(fd < 0) return FAIL;
	if(ioctl(fd,RTC_RD_TIME,&TimeRTC)){
		close(fd); 	return FAIL;
	}	close(fd);

	*buf++ = 0x51; *buf++ = 0x12; *buf++ = 0x80;
	*buf++ = 0x10; *buf++ = 0xA2; *buf++ = 0x04;
	/* system time */
	*buf++ = TimeSystem.tm_year%100;
	*buf++ = TimeSystem.tm_mon+1;
	*buf++ = TimeSystem.tm_mday;
	*buf++ = TimeSystem.tm_wday;
	*buf++ = TimeSystem.tm_hour;
	*buf++ = TimeSystem.tm_min;
	*buf++ = TimeSystem.tm_sec;
	/* get rtc time */
	*buf++ = TimeRTC.tm_year%100;
	*buf++ = TimeRTC.tm_mon+1;
	*buf++ = TimeRTC.tm_mday;
	*buf++ = TimeRTC.tm_wday;
	*buf++ = TimeRTC.tm_hour;
	*buf++ = TimeRTC.tm_min;
	*buf++ = TimeRTC.tm_sec;
	return SUCCESS;
}

int CC_Inquire_Version(u8 *buf,int size)
{
	assert_param(buf,FAIL);
	memset(buf,0,size);
	*buf++ = 0x51; *buf++ = 0x24; *buf++ = 0x80;
	*buf++ = 0x22; *buf++ = 0xA2; *buf++ = 0x05;
	sprintf((char*)buf,"%s",VERSION_NUMBER);
	return SUCCESS;
}

int SingleConfig(u8 *package,appitf_t *app)
{
	assert_param(package,FAIL);
	assert_param(app,FAIL);

	TableSingle_t   tab_single;
	tab_single.Wl_Addr 	= package[0];
	tab_single.lt_gid		= package[1];
	tab_single.Coor_id 		= package[2];
	tab_single.Base_Addr 	= (package[3] <<8) | package[4];
	tab_single.Map_Addr 	= (package[5] <<8) | package[6];
	debug(DEBUG_CC_Config,"table single:Wl_Addr=0x%02x,lt_gid=0x%02x,"\
		"Coor_id=0x%02x,Base_Addr=0x%04x,Map_Addr=0x%04x\n",tab_single.Wl_Addr,\
		tab_single.lt_gid,tab_single.Coor_id,tab_single.Base_Addr,tab_single.Map_Addr);

	if(SUCCESS != app->single->sin_config(app->single,cfg_sinGroup,&tab_single)) return FAIL;
	if(SUCCESS != app->single->sin_config(app->single,cfg_sinMap,&tab_single)) return FAIL;
	/* check sqlcolumn is exist */
	int res = -1;
	if(sql_isexist(app->sqlite,"db_light",tab_single.Base_Addr)){
		res = app->sqlite->sql_update("db_light",Asprintf("set Wl_Addr=%d,lt_gid=%d,"\
			"Coor_id=%d,Map_Addr=%d where Base_Addr=%d",tab_single.Wl_Addr,\
			tab_single.lt_gid,tab_single.Coor_id,tab_single.Map_Addr,tab_single.Base_Addr));
	}else{
		res = app->sqlite->sql_insert(Asprintf("insert into db_light(Wl_Addr,Base_Addr,lt_gid,"\
			"Coor_id,Map_Addr)  values(%d,0x%04X,%d,%d,%d);",tab_single.Wl_Addr,\
			tab_single.Base_Addr,tab_single.lt_gid,tab_single.Coor_id,tab_single.Map_Addr));
		if(SUCCESS == res)
			res = app->sqlite->sql_insert(Asprintf("insert into db_info_light(Base_Addr) "\
				"values(0x%04X);",tab_single.Base_Addr));
		if(SUCCESS != res)
			app->sqlite->sql_delete(Asprintf("delete from db_light where Base_Addr=%d;",tab_single.Base_Addr));
	}
	return res;
}

int CoordiConfig(u8 *package,  appitf_t *app)
{
	assert_param(package,FAIL);
	assert_param(app,FAIL);

	TableCoordi_t tab_coor;
	tab_coor.Wl_Addr 	= *package++;
	tab_coor.Coor_gid 	= *package++;
	memset(tab_coor.CC_id,0,sizeof(tab_coor.CC_id));
	Hex2Str(tab_coor.CC_id,package,6);package += 6 ;
	tab_coor.Base_Addr 	= *package++;
	tab_coor.Map_Addr 	= *package;

	debug(DEBUG_CC_Config,"CCUID: %s,Wl_Addr=0x%02x,Coor_gid=0x%02x,"\
		"Base_Addr=0x%02x,Map_Addr=0x%02x\n",tab_coor.CC_id,\
		tab_coor.Wl_Addr,tab_coor.Coor_gid,tab_coor.Base_Addr,tab_coor.Map_Addr);

	if(SUCCESS != app->single->sin_config(app->single,cfg_coorMap,&tab_coor)) return FAIL;

	if(sql_isexist(app->sqlite,"db_coordinator",tab_coor.Base_Addr)){
		return app->sqlite->sql_update("db_coordinator",Asprintf("set Wl_Addr=%d,Coor_gid=%d,"\
			"CC_id='%s',Map_Addr=%d where Base_Addr=%d",tab_coor.Wl_Addr,\
			tab_coor.Coor_gid,tab_coor.CC_id,tab_coor.Map_Addr,tab_coor.Base_Addr));
	}else{
		return app->sqlite->sql_insert( Asprintf("insert into db_coordinator(Wl_Addr,Base_Addr,"\
			"Coor_gid,CC_id,Map_Addr) values(%d,%d,%d,'%s',%d);",tab_coor.Wl_Addr,\
			tab_coor.Base_Addr,tab_coor.Coor_gid,tab_coor.CC_id,tab_coor.Map_Addr));
	}
}

int delete_sql(u8 *Pdata,appitf_t *app)
{
	assert_param(Pdata,FAIL);
	assert_param(app,FAIL);

	struct tm tim = {0};
	time_t start = 0,end = 0;

	switch(*Pdata++){
		case 0x00:		//协调器记录表
			if(*Pdata++ == 0x00){
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_coordinator where Base_Addr=0x%x;",*Pdata));
			}else{
				debug(DEBUG_DelSql,"db_coordinator have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		case 0x01:		//单灯记录表
			if(*Pdata++ == 0x00){
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_light where Base_Addr=0x%x;",*Pdata<<8 | *(Pdata+1)));
			}else{
				debug(DEBUG_DelSql,"db_light have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		case 0x02:		//任务表
			if(*Pdata == 0x01){//名称
				return app->sqlite->sql_delete(Asprintf("delete from db_task where Name=%s;",Pdata+1));
			}else if(*Pdata++ == 0x02){//时间范围
				Mktime(&tim,Pdata,start);
				Mktime(&tim,Pdata,end);
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_task where Start_Date < %ld AND Start_Date > %ld",end,start));
			}else{
				debug(DEBUG_DelSql,"db_task have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		case 0x03:		//报警日志记录表
			if(*Pdata++ == 0x02){//名称
				Mktime(&tim,Pdata,start);
				Mktime(&tim,Pdata,end);
				return app->sqlite->sql_delete(Asprintf(\
					"delete from db_warn where Start_Date < %ld AND Start_Date > %ld",end,start));
			}else{
				debug(DEBUG_DelSql,"db_warn have no coilmn type %d\n",*(Pdata-1));
				return FAIL;
			}	break;
		default:break;
	}
	return FAIL;
}
