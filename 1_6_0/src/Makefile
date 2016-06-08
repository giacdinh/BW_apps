CFLAGS= -ggdb -mfloat-abi=hard --sysroot=$(SYSROOT)
BINX=../install/binx

MACHINE:=$(shell uname -m)
CC=arm-poky-linux-gnueabi-gcc -v

INCLUDES=-I. -I$(TOOL_INC)/gstreamer-1.0 -I$(TOOL_INC) -I$(TOOL_INC)/glib-2.0 \
		 -I$(TOOL_INC)/libxml2 -I$(TOOL_INC)
L_LIBS=-L$(SYSROOT)usr/lib -lcap -lcrypt -lgcrypt -ltasn1 -lgpg-error -lgnutls -lcurl -ljson-c -lmosquitto

MQTT_DEF= -DWITH_TLS -DWITH_TLS_PSK

SUBDIRS = unittest

all: build_info monitor hashfile gst_capture

build_info:
	@echo "Build base machine: $(MACHINE)"
ezxml.o: 
	$(CC) $(INCLUDES) $(CFLAGS) -c ezxml.c

remotem.o: remotem.c odi-config.h version.h
	$(CC) $(CFLAGS) $(INCLUDES) -D_FILE_OFFSET_BITS=64 -c remotem.c

mongoose.o: odi-config.h
	$(CC) $(CFLAGS) $(INCLUDES) -D_FILE_OFFSET_BITS=64 -c mongoose.c

hashfile: hashfile.c Flash2AppSM_hash.c Flash2AppSM_hash.h
	$(CC) $(CFLAGS) -o hashfile Flash2AppSM_hash.c hashfile.c -lssl -lcrypto

Flash2AppSM.o:
	$(CC) $(CFLAGS) -c Flash2AppSM.c $(INCLUDES)

gst_capture: capture.c ezxml.o gst_capture.h odi-config.h Flash2AppSM_internal.h Flash2AppSM.o \
		capture_config.o capture_manager.o capture_session.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gst_capture capture.c ezxml.o Flash2AppSM.o \
		capture_config.o capture_manager.o capture_session.o $(LIBS) --sysroot=$(SYSROOT)

capture_config.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c capture_config.c

capture_manager.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c capture_manager.c

capture_session.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c capture_session.c
	
monitor: main.o monitor.o ezxml.o monitor.h mongoose.o remotem.o udp_util.o BVcloud.o \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o GetTime.o \
	RefreshToken.o mqtt_common.o mqtt_sub.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o Configupdate.o \
	LogGetSignedURL.o LogUpload.o monitor.o mvi_msg.o watchdog.o util.o gpio.o
	$(CC) $(CFLAGS) -o monitor -D_FILE_OFFSET_BITS=64 main.c monitor.o gpio.o \
		udp_util.o ezxml.o BVcloud.o FileUpload.o FileDownload.o GetSignedURL.o GetTime.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o Configupdate.o \
		mqtt_sub.o mqtt_common.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o \
		LogGetSignedURL.o LogUpload.o mvi_msg.o watchdog.o util.o mongoose.o remotem.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) --sysroot=$(SYSROOT)

#main.o: main.c
#	$(CC) $(CFLAGS) $(INCLUDES) -c main.c

monitor.o: monitor.c
	$(CC) $(CFLAGS) $(INCLUDES) -c monitor.c

util.o: util.c
	$(CC) $(CFLAGS) $(INCLUDES) -c util.c

gpio.o: gpio.c
	$(CC) $(CFLAGS) $(INCLUDES) -c gpio.c

udp_util.o: udp_util.c
	$(CC) $(CFLAGS) $(INCLUDES) -c udp_util.c

mvi_msg.o: mvi_msg.c
	$(CC) $(CFLAGS) $(INCLUDES) -c mvi_msg.c

watchdog.o: watchdog.c
	$(CC) $(CFLAGS) $(INCLUDES) -c watchdog.c

#Cloud build
BVcloud.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c BVcloud.c

FileUpload.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c FileUpload.c

FileDownload.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c FileDownload.c

GetSignedURL.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c GetSignedURL.c 

GetToken.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c GetToken.c

Parse_Json.o:
	$(CC) $(CFLAGS) $(INCLUDES) -ggdb -std=c99 -c Parse_Json.c

UpdateStatus.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c UpdateStatus.c 

RefreshToken.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c RefreshToken.c 

GetTime.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c GetTime.c 

Configupdate.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c Configupdate.c 

LogGetSignedURL.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c LogGetSignedURL.c 

LogUpload.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c LogUpload.c 

#Cloud config
mqtt_sub.o : mqtt_sub.c
	$(CC) $(CFLAGS) $(INCLUDES) -c mqtt_sub.c $(MQTT_DEF)

mqtt_sub_init.o : mqtt_sub_init.c
	$(CC) $(CFLAGS) $(INCLUDES) -c mqtt_sub_init.c $(MQTT_DEF)

mqtt_pub.o : mqtt_pub.c
	$(CC) $(CFLAGS) $(INCLUDES) -c mqtt_pub.c $(MQTT_DEF)

mqtt_common.o: mqtt_common.c
	$(CC) $(CFLAGS) $(INCLUDES) -c mqtt_common.c $(MQTT_DEF)

mqtt_token.o: mqtt_token.c
	$(CC) $(CFLAGS) $(INCLUDES) -c mqtt_token.c

pub.o: pub.c
	$(CC) $(CFLAGS) $(INCLUDES) -c pub.c

clean:
	rm -f gst_capture monitor hashfile *.o
	
install:
	cd unittest; make; make install; cd ..
	cp gst_capture $(BINX)
	cp monitor $(BINX)
	cp hashfile $(BINX)

install-gst_capture:
	cp gst_capture $(BINX)

install-monitor:
	cp monitor $(BINX)
