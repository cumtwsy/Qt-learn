#include"data_collect.h"

unsigned char modbus1[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
unsigned char modbus2[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x3a};
unsigned char modbus3[8] = {0x03, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0xeb};
unsigned char modbus4[8] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x5c};
unsigned char modbus5[8] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0x8d};
unsigned char modbus6[8] = {0x06, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0xbe};
unsigned char modbus7[8] = {0x03, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4F, 0x24};
unsigned char modbus8[8] = {0x03, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4E, 0xA3};
unsigned char get_modbus1[8] = {0x00};
unsigned char get_modbus2[8] = {0x00};
unsigned char get_modbus3[8] = {0x00};
unsigned char get_modbus4[8] = {0x00};
unsigned char get_modbus5[8] = {0x00};
unsigned char get_modbus6[8] = {0x00};
unsigned char get_modbus7[8] = {0x00};
unsigned char get_modbus8[8] = {0x00};

nmea_msg gprs;
Str_RECORD data_record;

int time_tick=1000;



unsigned long string_to_int_d(unsigned char *u16Byte, unsigned char len)
{
    union FloattByte //联合体中的量占用同存储空间
    {
        unsigned char charbuffer[4];
        unsigned long TEST_DATA;
    } comm_u16;
    comm_u16.TEST_DATA = 0;
    switch (len)
    {
    case 4:
        comm_u16.charbuffer[0] = u16Byte[3];
        comm_u16.charbuffer[1] = u16Byte[2];
        comm_u16.charbuffer[2] = u16Byte[1];
        comm_u16.charbuffer[3] = u16Byte[0];
        break;
    case 3:
        comm_u16.charbuffer[0] = u16Byte[2];
        comm_u16.charbuffer[1] = u16Byte[1];
        comm_u16.charbuffer[2] = u16Byte[0];
        break;
    case 2:
        comm_u16.charbuffer[0] = u16Byte[1];
        comm_u16.charbuffer[1] = u16Byte[0];
        break;
    case 1:
        comm_u16.charbuffer[0] = u16Byte[0];
        break;
    }
    return comm_u16.TEST_DATA;
}

void int_to_string(unsigned char *u16Byte, unsigned char len, unsigned long shu) //小端模式
{
    union FloattByte //联合体中的量占用同存储空间
    {
        unsigned char charbuffer[4];
        unsigned long TEST_DATA;
    } comm_u16;
    comm_u16.TEST_DATA = shu;
    switch (len)
    {
    case 4:
        u16Byte[3] = comm_u16.charbuffer[3];
    case 3:
        u16Byte[2] = comm_u16.charbuffer[2];
    case 2:
        u16Byte[1] = comm_u16.charbuffer[1];
    case 1:
        u16Byte[0] = comm_u16.charbuffer[0];
        break;
    }
}


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

void data_collect()
{
    int fd;
    int ret=0;
    int timeout=0;
    int parameter=0;
    int water_temperature=0;
    //int air_temperature=0;
    fd=open("/dev/ttyS2",O_RDWR|O_NOCTTY);
    while(fd<0)
    {
        perror("open usart device error. \n");
        usleep(time_tick);
        fd=open("/dev/ttyS2",O_RDWR|O_NOCTTY);
        timeout++;
        if(timeout>3)
        {
            break;
        }
    }
    timeout=0;
    write(fd, modbus1, sizeof(modbus1));
    usleep(time_tick);
    while(read(fd, get_modbus1, sizeof(get_modbus1))<0&&read(fd, get_modbus1, sizeof(get_modbus1))==0)
    {
        write(fd, modbus1, sizeof(modbus1));
        usleep(time_tick);
        timeout++;
        if(timeout>3)
        {
            ret=1;
            break;
        }
        if(ret==0)
        {
            parameter = string_to_int_d(&get_modbus1[3], 2);
            int_to_string(data_record.diandaolv, 3, parameter);
        }
        else
        {
            int_to_string(data_record.diandaolv, 3, 9999);
        }
    }
    write(fd, modbus2, sizeof(modbus2));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus2, sizeof(get_modbus2))<0&&read(fd, get_modbus2, sizeof(get_modbus2))==0)
    {
        write(fd, modbus2, sizeof(modbus2));
        usleep(time_tick);
        timeout++;
        if(timeout>3)
        {
            ret=1;
            break;
        }
        if(ret==0)
        {
            parameter = string_to_int_d(&get_modbus2[3], 2);
            int_to_string(data_record.ph, 2, parameter);
            water_temperature = string_to_int_d(&get_modbus2[7], 2); //  250   25度
            int_to_string(data_record.wendu, 2, 1000 +water_temperature);
        }
        else
        {
            int_to_string(data_record.ph,2,9999);
            int_to_string(data_record.wendu,2,9999);
        }
    }
    write(fd, modbus3, sizeof(modbus3));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus3, sizeof(get_modbus3))<0&&read(fd, get_modbus3, sizeof(get_modbus3))==0)
    {
        write(fd, modbus3, sizeof(modbus3));
        usleep(time_tick);
        timeout++;
        if(timeout>3)
        {
            ret=1;
            break;
        }
        if(ret==0&&get_modbus3[3]==0x03)
        {
            parameter = string_to_int_d(&get_modbus3[3], 2);
            int_to_string(data_record.rongjieyang, 3, parameter);
        }
        else
        {
            int_to_string(data_record.rongjieyang,3,9999);
        }
    }
    write(fd, modbus4, sizeof(modbus4));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus4, sizeof(get_modbus4))<0&&read(fd, get_modbus4, sizeof(get_modbus4))==0)
    {
        write(fd, modbus4, sizeof(modbus4));
        usleep(time_tick);
        timeout++;
        if(timeout>3)
        {
            ret=1;
            break;
        }
        if(ret==0)
        {
            parameter = string_to_int_d(&get_modbus4[3], 2)*10;
            int_to_string(data_record.zhuodu, 3, parameter);
        }
        else
        {
            int_to_string(data_record.zhuodu,3,9999);
        }
    }
    write(fd, modbus5, sizeof(modbus5));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus5, sizeof(get_modbus5))<0&&read(fd, get_modbus5, sizeof(get_modbus5))==0)
    {
        write(fd, modbus5, sizeof(modbus5));
        usleep(time_tick);
        timeout++;
        if(timeout>3)
        {
            ret=1;
            break;
        }
        if(ret==0)
        {
            parameter = string_to_int_d(&get_modbus5[3], 2)*10;
            int_to_string(data_record.andan, 3, parameter);
        }
        else
        {
            int_to_string(data_record.andan,3,9999);
        }
    }
    write(fd, modbus6, sizeof(modbus6));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus6, sizeof(get_modbus6))<0&&read(fd, get_modbus6, sizeof(get_modbus6))==0)
    {
        write(fd, modbus6, sizeof(modbus6));
        usleep(time_tick);
        timeout++;
        if(timeout>3)
        {
            ret=1;
            break;
        }
        if(ret==0)
        {
            parameter = string_to_int_d(&get_modbus6[3], 2)*10;
            int_to_string(data_record.cod, 3, parameter);
        }
        else
        {
            int_to_string(data_record.cod,3,9999);
        }
    }


}
