#ifndef DATA_COLLECT_H
#define DATA_COLLECT_H

#include <string.h>
#include <termios.h>

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

int data_collect(void);




#endif // DATA_COLLECT_H

