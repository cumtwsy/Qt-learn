#ifndef DATA_COLLECT_H
#define DATA_COLLECT_H

#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct
{
    unsigned char start;		   //数据开始
    unsigned char datetime[8];	   //当前日期时间	BYTE[8]	    年（2011 表示：第一个字节，0xDB第二个字节：0x07）
    unsigned char diandaolv[3];   //电导率	BYTE[3]	3		按下面公式计算后为正整数；
    unsigned char ph[2];		   //PH	BYTE[2]	2		    按下面公式计算后为正整数；
    unsigned char wendu[2];	   //温度	BYTE[2]	2		按下面公式计算后为正整数；(℃)实际值*100
    unsigned char rongjieyang[3]; //溶解氧	BYTE[3]	3		按下面公式计算后为正整数；(mg/L)实际值*100
    unsigned char zhuodu[3];	   //浊度	BYTE[3]	3		按下面公式计算后为正整数；(度)实际值*100
    unsigned char andan[3];	   //氨氮	BYTE[3]	3		按下面公式计算后为正整数；(mg/L)实际值*100
    unsigned char cod[3];		   //COD	BYTE[3]	3		按下面公式计算后为正整数；(mg/L)实际值*100
    unsigned char qiwen[2];	   //空气温度	BYTE[2]	2		按下面公式计算后为正整数；(℃)实际值*100
    unsigned char end;            //数据结束
    unsigned char crc;
} Str_RECORD; //32B

typedef struct
{
    int diandaolv;
    int ph;
    int wendu;
    int rongjieyang;
    int zhuodu;
    int andan;
    int cod;
    int qiwen;
} DATA;

typedef struct  
{										    
 	unsigned short year;	//年份
	unsigned char month;	//月份
	unsigned char date;	    //日期
	unsigned char hour; 	//小时
	unsigned char min; 	    //分钟
	unsigned char sec; 	    //秒钟
}nmea_utc_time;


typedef struct  
{										    
	nmea_utc_time utc;                      //UTC时间
	unsigned long latitude;				    //纬度 分扩大100000倍,实际要除以100000
	unsigned char nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	unsigned long longitude;			    //经度 分扩大100000倍,实际要除以100000
	unsigned char ewhemi;					//东经/西经,E:东经;W:西经
}nmea_msg;


void NMEA_GPRMC_Analysis(nmea_msg *gpsx, unsigned char *buf);

void data_collect(void);

DATA data_collect2(void);

nmea_msg get_gps(void);

unsigned long string_to_int_d(unsigned char *u16Byte, unsigned char len);

int openSerial(int iPort);

int serial_init(void);

int fd;

int fd2;


#endif // DATA_COLLECT_H

