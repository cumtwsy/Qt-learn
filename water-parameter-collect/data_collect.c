#include"data_collect.h"




unsigned char modbus1[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
unsigned char modbus2[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x3a};
unsigned char modbus3[8] = {0x03, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0xeb};
unsigned char modbus4[8] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x5c};
unsigned char modbus5[8] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0x8d};
unsigned char modbus6[8] = {0x06, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0xbe};
unsigned char modbus7[8] = {0x03, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4F, 0x24};
unsigned char modbus8[8] = {0x03, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4E, 0xA3};

nmea_msg gprs;



/***********************************
 * 从buf里面得到第cx个逗号所在的位置
 * 返回值:0~0XFE,代表逗号所在位置的偏移.
 * 0XFF,代表不存在第cx个逗号
 * ********************************/
unsigned char NMEA_Comma_Pos(unsigned char *buf, unsigned char cx)
{
    unsigned char *p = buf;
	while (cx)
	{
		if (*buf == '*' || *buf < ' ' || *buf > 'z')
			return 0XFF; //遇到'*'或者非法字符,则不存在第cx个逗号
		if (*buf == ',')
			cx--;
		buf++;
	}
	return buf - p;
}

/**************
 * m^n函数
 * 返回值:m^n次方.
 * ***********/
unsigned long NMEA_Pow(unsigned char m, unsigned char n)
{
    unsigned long result = 1;
	while (n--)
		result *= m;
	return result;
}


/*****************************
 * str转换为数字,以','或者'*'结束
 * buf:数字存储区
 * dx:小数点位数,返回给调用函数
 * 返回值:转换后的数值
 * **************************/
int NMEA_Str2num(unsigned char *buf, unsigned char *dx)
{
	unsigned char *p = buf;
    unsigned long ires = 0, fres = 0;
	unsigned char ilen = 0, flen = 0, i;
	unsigned char mask = 0;
	int res;
	while (1) //得到整数和小数的长度
	{
		if (*p == '-')
		{
			mask |= 0X02;
			p++;
		} //是负数
		if (*p == ',' || (*p == '*'))
			break; //遇到结束了
		if (*p == '.')
		{
			mask |= 0X01;
			p++;
		}								 //遇到小数点了
		else if (*p > '9' || (*p < '0')) //有非法字符
		{
			ilen = 0;
			flen = 0;
			break;
		}
		if (mask & 0X01)
			flen++;
		else
			ilen++;
		p++;
	}
	if (mask & 0X02)
		buf++;				   //去掉负号
	for (i = 0; i < ilen; i++) //得到整数部分数据
	{
		ires += NMEA_Pow(10, ilen - 1 - i) * (buf[i] - '0');
	}
	if (flen > 5)
		flen = 5;			   //最多取5位小数
	*dx = flen;				   //小数点位数
	for (i = 0; i < flen; i++) //得到小数部分数据
	{
		fres += NMEA_Pow(10, flen - 1 - i) * (buf[ilen + 1 + i] - '0');
	}
	res = ires * NMEA_Pow(10, flen) + fres;
	if (mask & 0X02)
		res = -res;
	return res;
}


/*************************************
*分析GPRMC信息
*gpsx:nmea信息结构体
*buf:接收到的GPS数据缓冲区首地址
*************************************/
void NMEA_GPRMC_Analysis(nmea_msg *gpsx, unsigned char *buf)
{
	unsigned char *p1, dx;
	unsigned char posx;
	unsigned long temp;
	float rs;
	p1 = (unsigned char *)strstr((const char *)buf, "GPRMC"); //"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
	posx = NMEA_Comma_Pos(p1, 1);				   //得到UTC时间
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx) / NMEA_Pow(10, dx); //得到UTC时间,去掉ms
		gpsx->utc.hour = temp / 10000;
		gpsx->utc.min = (temp / 100) % 100;
		gpsx->utc.sec = temp % 100;
	}
	posx = NMEA_Comma_Pos(p1, 3); //得到纬度
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx);
		gpsx->latitude = temp / NMEA_Pow(10, dx + 2);										  //得到°
		rs = temp % NMEA_Pow(10, dx + 2);													  //得到'
		gpsx->latitude = gpsx->latitude * NMEA_Pow(10, 6) + (rs * NMEA_Pow(10, 6 - dx)) / 60; //转换为°
	}
	posx = NMEA_Comma_Pos(p1, 4); //南纬还是北纬
	if (posx != 0XFF)
		gpsx->nshemi = *(p1 + posx);
	posx = NMEA_Comma_Pos(p1, 5); //得到经度
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx);
		gpsx->longitude = temp / NMEA_Pow(10, dx + 2);											//得到°
		rs = temp % NMEA_Pow(10, dx + 2);														//得到'
		gpsx->longitude = gpsx->longitude * NMEA_Pow(10, 6) + (rs * NMEA_Pow(10, 6 - dx)) / 60; //转换为°
	}
	posx = NMEA_Comma_Pos(p1, 6); //东经还是西经
	if (posx != 0XFF)
		gpsx->ewhemi = *(p1 + posx);
	posx = NMEA_Comma_Pos(p1, 9); //得到UTC日期
	if (posx != 0XFF)
	{
		temp = NMEA_Str2num(p1 + posx, &dx); //得到UTC日期
		gpsx->utc.date = temp / 10000;
		gpsx->utc.month = (temp / 100) % 100;
		gpsx->utc.year = 2000 + temp % 100;
	}
}
