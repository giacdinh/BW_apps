DEFS=-DHAVE_LIBGSTREAMER_0_10=1 -DHAVE_LIBXML2=1 -DHAVE_LIBCRYPTO=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 \
 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 \
 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STDLIB_H=1 -DHAVE_OPENSSL_SHA_H=1 -DHAVE_SETENV=1

CFLAGS=-O2 -ggdb
LIBS= -lxml2 -lm -lgobject-2.0 -lglib-2.0 -lgstreamer-0.10 -lssl -lcrypto
BINX=../../install/binx

MACHINE:=$(shell uname -m)
L_LIBS=-L../../usr/lib -lcap -lcrypt -lgcrypt -ltasn1 -lgpg-error -lgnutls -lcurl -ljson \
	../../usr/lib/libmosquitto.so.1 ../../usr/lib/libcares.so.2

ifeq ($(MACHINE),i686)
INCLUDES=-I. -I$(TOOL_INC)/gstreamer-0.10 -I$(TOOL_INC)
INCLUDES+= -I$(TOOL_INC)/glib-2.0 -I$(TOOL_INC)/libxml2 -I../../usr/include
CC=arm-poky-linux-gnueabi-gcc
else
INCLUDES=-I. -I/usr/include/gstreamer-0.10 -I/usr/include
INCLUDES+= -I/usr/include/glib-2.0 -I/usr/include/libxml2 
CC=gcc
endif
MQTT_DEF= -DWITH_TLS -DWITH_TLS_PSK

all: aws_gettoken aws_getsignedurl aws_putfile aws_update uC_test aws_gettime

aws_gettoken: \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o \
	RefreshToken.o misc.o
	$(CC) $(CFLAGS) -o aws_gettoken aws_gettoken.c misc.o \
		FileUpload.o FileDownload.o GetSignedURL.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

aws_getsignedurl: \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o \
	RefreshToken.o misc.o
	$(CC) $(CFLAGS) -o aws_getsignedurl aws_getsignedurl.c misc.o \
		FileUpload.o FileDownload.o GetSignedURL.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 
aws_getfile: \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o \
	RefreshToken.o misc.o
	$(CC) $(CFLAGS) -o aws_getfile aws_getfile.c misc.o \
		FileUpload.o FileDownload.o GetSignedURL.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

aws_putfile: \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o \
	RefreshToken.o misc.o
	$(CC) $(CFLAGS) -o aws_putfile aws_putfile.c misc.o \
		FileUpload.o FileDownload.o GetSignedURL.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

aws_update: \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o \
	RefreshToken.o misc.o
	$(CC) $(CFLAGS) -o aws_update aws_update.c misc.o \
		FileUpload.o FileDownload.o GetSignedURL.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 
aws_gettime: \
	FileUpload.o FileDownload.o GetSignedURL.o GetToken.o Parse_Json.o UpdateStatus.o \
	RefreshToken.o misc.o
	$(CC) $(CFLAGS) $(INCLUDES) -o aws_gettime aws_gettime.c misc.o \
		FileUpload.o FileDownload.o GetSignedURL.o \
		GetToken.o Parse_Json.o  UpdateStatus.o RefreshToken.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

mqtt_gettoken: \
	mqtt_common.o mqtt_sub.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o Parse_Json.o
	$(CC) $(CFLAGS) -o mqtt_gettoken mqtt_gettoken.c \
		Parse_Json.o mqtt_sub.o mqtt_common.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

mqttinitsub: \
	mqtt_common.o mqtt_sub.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o Parse_Json.o
	$(CC) $(CFLAGS) -o mqttinitsub mqttinitsub.c \
		Parse_Json.o mqtt_sub.o mqtt_common.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

mqttsub: \
	mqtt_common.o mqtt_sub.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o Parse_Json.o
	$(CC) $(CFLAGS) -o mqttsub mqttsub.c \
		Parse_Json.o mqtt_sub.o mqtt_common.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

mqttpub: \
	mqtt_common.o mqtt_sub.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o Parse_Json.o
	$(CC) $(CFLAGS) -o mqttpub mqttpub.c \
		Parse_Json.o mqtt_sub.o mqtt_common.o mqtt_sub_init.o mqtt_token.o mqtt_pub.o \
		-lcrypto -lxml2 -lpthread -ldl -pipe $(L_LIBS) 

FileUpload.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../FileUpload.c

FileDownload.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../FileDownload.c

GetSignedURL.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GetSignedURL.c 

GetToken.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GetToken.c

Parse_Json.o:
	$(CC) -ggdb -std=c99 -I../../usr/include -c ../Parse_Json.c

UpdateStatus.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../UpdateStatus.c 

RefreshToken.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../RefreshToken.c 

GetTime.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GetTime.c 

#Cloud config
mqtt_sub.o :
	$(CC) $(CFLAGS) $(INCLUDES) -c ../mqtt_sub.c $(MQTT_DEF)

mqtt_sub_init.o :
	$(CC) $(CFLAGS) $(INCLUDES) -c ../mqtt_sub_init.c $(MQTT_DEF)

mqtt_pub.o :
	$(CC) $(CFLAGS) $(INCLUDES) -c ../mqtt_pub.c $(MQTT_DEF)

mqtt_common.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../mqtt_common.c $(MQTT_DEF)

mqtt_token.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c ../mqtt_token.c

misc.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c misc.c

#Tools and unit test
uC_test:
	$(CC) $(CFLAGS) $(INCLUDES) -o uC_test uC_test.c

clean:
	rm -f *.o aws_update aws_gettoken aws_getsignedurl aws_putfile uC_test

install:
	cp aws_update $(BINX)
	cp aws_gettoken $(BINX)
	cp aws_gettime $(BINX)
	cp aws_getsignedurl $(BINX)
	cp aws_putfile $(BINX)
	cp uC_test $(BINX)
	cp pub_prod.sh $(BINX)
	cp sub_prod.sh $(BINX)
