CC = arm-linux-emfure-gcc
#CC = arm-linux-gcc
CFLAGS =  -Wall
CFLAGS += -Wno-implicit-function-declaration
#关闭由-O2优化选项带来的"警告"提示
CFLAGS += -Wno-strict-aliasing
#关闭数据包data数组越界的"警告"提示
CFLAGS += -Wno-array-bounds

LDFLAGS = -L$(PWD)/sqlite/lib -L$(PWD)/Software/emfuture/lib  -lsqlite3 -lEM_Middleware_Lib
#LDFLAGS = -L$(PWD)/sqlite/lib   -lsqlite3
#LDFLAGS = -L/usr/local/sqlite3/lib/   -lsqlite3

LINKFLAGS = -lpthread
DFLAGS =

OBJS :=
output := output
Rootdir=$(PWD)
export output CFLAGS Rootdir

#OBJS_PATH := $(shell find -maxdepth 1 -type d -name '*' | grep '[a-z]')
OBJS_PATH +=./Common/common ./Common ./Common/My_lib ./hardware/coordinate ./hardware/ethernet ./hardware/gprs_ppp 	\
	./hardware/serial ./Software/analysis ./Software/burglar_alarm ./Software/cc_param ./Software/communication 	\
	./Software/info_output ./Software/initstart ./Software/link_method ./Software/Log ./Software/meter_crc 		\
	./Software/cmd_process ./sqlite ./task/tasklist ./User ./hardware/update ./task/timertask  ./hardware/Zt_Meter


#包含子目录的工程文件
INCLUDE := $(addsuffix /Makefile, $(OBJS_PATH))
include $(INCLUDE)

DESTOBJS := $(addprefix ./$(output)/, $(notdir $(OBJS)))

all: Version ztcc Automountusb  Watchrsh

ztcc: compile
	@$(CC)  $(DESTOBJS) -o ./Applications/$@ $(LINKFLAGS) $(LDFLAGS)
Version:
	@`./version/setlocalversion`

Automountusb:
	@$(CC) -o  ./Applications/$@  ./tools/AutoMountUsb.c
Watchrsh:
	@$(CC) -o ./Applications/$@ ./tools/Watch_ssh.c


#编译子目录的c文件
compile:
	@$(foreach N,$(OBJS_PATH),make -C $(N) CC=$(CC);)

#将可程序拷贝到nfs目录下
install:
	@cp -frd ./Applications/*  $(PRJROOT)/rootfs/
	@cp -frd ./config/ $(PRJROOT)/rootfs/


svnver:
	@echo "#ifndef _H_VWESION_H_" > ./version/version.h
	@echo "#define _H_VWESION_H_" >> ./version/version.h
	@echo "#define VERSION \"(0.2) `./version/setlocalversion`\"" >> ./version/version.h
	@echo "#endif" >> ./version/version.h

tar:
	tar -zcvf flash.gz autorun.sh ./bin/test_App
	tar -zcvf conf.gz term.ini net_params.conf

version:
	./makeversion


.PHONY: clean
clean:
	@find -type f \( -name '*.o' \) -print | xargs rm -f
distclean:clean
	@rm ./bin/test_App -f

