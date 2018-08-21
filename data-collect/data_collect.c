#include"data_collect.h"

#define gpsfilename "/opt/gps.txt"

unsigned char modbus1[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
unsigned char modbus2[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x3a};
unsigned char modbus3[8] = {0x03, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0xeb};
unsigned char modbus4[8] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x5c};
unsigned char modbus5[8] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0x8d};
unsigned char modbus6[8] = {0x06, 0x03, 0x00, 0x00, 0x00, 0x04, 0x45, 0xbe};
unsigned char modbus7[8] = {0x03, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4F, 0x24};
unsigned char modbus8[8] = {0x03, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4E, 0xA3};
unsigned char get_modbus1[13] = {0x00};
unsigned char get_modbus2[13] = {0x00};
unsigned char get_modbus3[13] = {0x00};
unsigned char get_modbus4[13] = {0x00};
unsigned char get_modbus5[13] = {0x00};
unsigned char get_modbus6[13] = {0x00};
unsigned char get_modbus7[13] = {0x00};
unsigned char get_modbus8[13] = {0x00};
unsigned char get_gps_buf[2000]={0x00};

nmea_msg gps;
Str_RECORD data_record;
DATA data_record2;

int time_tick=1000;

static int GiSerialFds[4] = {-1, -1, -1, -1};

int openSerial(int iPort)
{
    int iFd;

    struct termios opt;
    char cSerialName[15];

    if (iPort >= 10) {
        printf("no this serial:ttySP%d . \n", iPort);
        exit(1);
    }

    if (GiSerialFds[iPort - 1] > 0) {
        return GiSerialFds[iPort - 1];
    }

    sprintf(cSerialName, "/dev/ttySP%d", iPort - 1);
    printf("open serila name:%s \n", cSerialName);
    iFd = open(cSerialName, O_RDWR | O_NOCTTY);
    if(iFd < 0) {
        perror(cSerialName);
        return -1;
    }

    tcgetattr(iFd, &opt);

    cfsetispeed(&opt, B9600);
    cfsetospeed(&opt, B9600);

     //cfsetispeed(&opt, B115200);
     //cfsetospeed(&opt, B115200);


    /*
     * raw mode
     */
    opt.c_lflag   &=   ~(ECHO   |   ICANON   |   IEXTEN   |   ISIG);
    opt.c_iflag   &=   ~(BRKINT   |   ICRNL   |   INPCK   |   ISTRIP   |   IXON);
    opt.c_oflag   &=   ~(OPOST);
    opt.c_cflag   &=   ~(CSIZE   |   PARENB);
    opt.c_cflag   |=   CS8;

    /*
     * 'DATA_LEN' bytes can be read by serial
     */
    opt.c_cc[VMIN]   =   13;
    opt.c_cc[VTIME]  =   100;
    if (tcsetattr(iFd,   TCSANOW,   &opt)<0) {
        return   -1;
    }

    GiSerialFds[iPort - 1] = iFd;

    return iFd;
}

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
    //int fd;
    int len=0;
    int ret=0;
    int timeout=0;
    int parameter=0;
    int water_temperature=0;
    //int air_temperature=0;
    /*fd=openSerial(3);
    while(fd<0)
    {
        perror("open usart device error. \n");
        usleep(time_tick);
        fd=open("/dev/ttySP2",O_RDWR|O_NOCTTY);
        timeout++;
        if(timeout>3)
        {
            printf("error to open the rs485.\n");
            break;
        }
    }*/
    timeout=0;
    len=write(fd, modbus3, sizeof(modbus3));
    usleep(time_tick);
    if(len<0)
    {
        printf("write data failure.\n");
    }
    else
    {
        printf("write data success.\n");
    }
    while(read(fd, get_modbus3, sizeof(get_modbus3))<0)
    {
        write(fd,modbus3,sizeof(modbus3));
        timeout++;
        printf("resend the modbus3.\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
    if(ret==0)
    {
        printf("read the get_modbus1 success!\n");
        parameter = string_to_int_d(&get_modbus3[3], 2);
        printf("get data is %s .\n", get_modbus3);
        printf("rongjieyang is %d.\n",parameter);
        int_to_string(data_record.rongjieyang, 3, parameter);
    }
    else
    {
        printf("read the get_modbus1 failure!\n");
        int_to_string(data_record.rongjieyang,3,9999);
    }

    write(fd, modbus2, sizeof(modbus2));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus2, sizeof(get_modbus2))<0)
    {
        write(fd, modbus2, sizeof(modbus2));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus2\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
        if(ret==0)
        {
            printf("read the get_modbus2 success!\n");
            printf("%s \n", get_modbus2);
            parameter = string_to_int_d(&get_modbus2[3], 2);
            printf("ph is %d.\n",parameter);
            int_to_string(data_record.ph, 2, parameter);
            water_temperature = string_to_int_d(&get_modbus2[7], 2); //  250   25度
            printf("water_temperture is %d.\n",water_temperature);
            int_to_string(data_record.wendu, 2, 1000 +water_temperature);
        }
        else
        {
            printf("read the get_modbus2 failure!\n");
            int_to_string(data_record.ph,2,9999);
            int_to_string(data_record.wendu,2,9999);
        }

    write(fd, modbus1, sizeof(modbus1));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus1, sizeof(get_modbus1))<0)
    {
        write(fd, modbus1, sizeof(modbus1));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus3\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
        if(ret==0)
        {
            printf("read the get_modbus1 success!\n");
            printf("%s \n", get_modbus1);
            parameter = string_to_int_d(&get_modbus1[3], 2);
            printf("diandaolv is %d.\n",parameter);
            int_to_string(data_record.diandaolv, 3, parameter);
        }
        else
        {
            printf("read the get_modbus3 failure!\n");
            int_to_string(data_record.rongjieyang,3,9999);
        }

    write(fd, modbus4, sizeof(modbus4));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus4, sizeof(get_modbus4))<0)
    {
        write(fd, modbus4, sizeof(modbus4));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus4\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }

   }
        if(ret==0)
        {
            printf("read the get_modbus4 success!\n");
            printf("%s \n", get_modbus4);
            parameter = string_to_int_d(&get_modbus4[3], 2)*10;
            printf("zhuodu is %d.\n",parameter);
            int_to_string(data_record.zhuodu, 3, parameter);
        }
        else
        {
            printf("read the get_modbus4 failure!\n");
            int_to_string(data_record.zhuodu,3,9999);
        }

    write(fd, modbus5, sizeof(modbus5));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus5, sizeof(get_modbus5))<0)
    {
        write(fd, modbus5, sizeof(modbus5));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus5\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
   }
        if(ret==0)
        {
            printf("read the get_modbus5 success!\n");
            printf("%s \n", get_modbus5);
            parameter = string_to_int_d(&get_modbus5[3], 2)*10;
            printf("andan is %d.\n",parameter);
            int_to_string(data_record.andan, 3, parameter);
        }
        else
        {
            printf("read the get_modbus5 failure!\n");
            int_to_string(data_record.andan,3,9999);
        }

    /*write(fd, modbus6, sizeof(modbus6));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus6, sizeof(get_modbus6))<0)
    {
        write(fd, modbus6, sizeof(modbus6));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus6\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
        if(ret==0)
        {
            printf("read the get_modbus6 success!\n");
            parameter = string_to_int_d(&get_modbus6[3], 2)*10;
            printf("cod is %d.\n",parameter);
            int_to_string(data_record.cod, 3, parameter);
        }
        else
        {
            printf("read the get_modbus6 failure!\n");
            int_to_string(data_record.cod,3,9999);
        }*/


    //close(fd);

}

DATA data_collect2()
{
    //int fd;
    int len=0;
    int ret=0;
    int timeout=0;
    int parameter=0;
    int water_temperature=0;

    timeout=0;
    len=write(fd, modbus3, sizeof(modbus3));
    usleep(time_tick);
    if(len<0)
    {
        printf("write data failure.\n");
    }
    else
    {
        printf("write data success.\n");
    }
    while(read(fd, get_modbus3, sizeof(get_modbus3))<0)
    {
        write(fd,modbus3,sizeof(modbus3));
        timeout++;
        printf("resend the modbus3.\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
    if(ret==0)
    {
        printf("read the get_modbus1 success!\n");
        parameter = string_to_int_d(&get_modbus3[3], 2);
        printf("get data is %s .\n", get_modbus3);
        printf("rongjieyang is %d.\n",parameter);
        data_record2.rongjieyang=parameter;
    }
    else
    {
        printf("read the get_modbus1 failure!\n");
        data_record2.rongjieyang=9999;
    }

    write(fd, modbus2, sizeof(modbus2));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus2, sizeof(get_modbus2))<0)
    {
        write(fd, modbus2, sizeof(modbus2));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus2\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
        if(ret==0)
        {
            printf("read the get_modbus2 success!\n");
            printf("get data is %s \n", get_modbus2);
            parameter = string_to_int_d(&get_modbus2[3], 2);
            printf("ph is %d.\n",parameter);
            data_record2.ph=parameter;
            water_temperature = string_to_int_d(&get_modbus2[7], 2); //  250   25度
            printf("water_temperture is %d.\n",water_temperature);
            data_record2.wendu=water_temperature;
        }
        else
        {
            printf("read the get_modbus2 failure!\n");
            data_record2.ph=9999;
            data_record2.wendu=9999;
        }

    write(fd, modbus1, sizeof(modbus1));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus1, sizeof(get_modbus1))<0)
    {
        write(fd, modbus1, sizeof(modbus1));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus3\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
    }
        if(ret==0)
        {
            printf("read the get_modbus1 success!\n");
            printf("%s \n", get_modbus1);
            parameter = string_to_int_d(&get_modbus1[3], 2);
            printf("diandaolv is %d.\n",parameter);
            data_record2.diandaolv=parameter;
        }
        else
        {
            printf("read the get_modbus3 failure!\n");
            data_record2.diandaolv=9999;
        }

    write(fd, modbus4, sizeof(modbus4));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus4, sizeof(get_modbus4))<0)
    {
        write(fd, modbus4, sizeof(modbus4));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus4\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }

   }
        if(ret==0)
        {
            printf("read the get_modbus4 success!\n");
            printf("%s \n", get_modbus4);
            parameter = string_to_int_d(&get_modbus4[3], 2)*10;
            printf("zhuodu is %d.\n",parameter);
            data_record2.zhuodu=parameter;
        }
        else
        {
            printf("read the get_modbus4 failure!\n");
            data_record2.zhuodu=9999;
        }

    write(fd, modbus5, sizeof(modbus5));
    usleep(time_tick);
    timeout = 0;
    ret = 0;
    while(read(fd, get_modbus5, sizeof(get_modbus5))<0)
    {
        write(fd, modbus5, sizeof(modbus5));
        usleep(time_tick);
        timeout++;
        printf("resend the modbus5\n");
        if(timeout>3)
        {
            ret=1;
            break;
        }
   }
        if(ret==0)
        {
            printf("read the get_modbus5 success!\n");
            printf("%s \n", get_modbus5);
            parameter = string_to_int_d(&get_modbus5[3], 2)*10;
            printf("andan is %d.\n",parameter);
            data_record2.andan=parameter;
        }
        else
        {
            printf("read the get_modbus5 failure!\n");
            data_record2.andan=9999;
        }

    return data_record2;
}

nmea_msg get_gps(void)
{
    int read_time=0;
    int read_time2=5;
    char buf1[10]={0};
    char buf2[10]={0};
    FILE *f;
    unsigned long shuju1=0,shuju2=0;
    f=fopen("gpsfilename","w");
    while(read(fd2, get_gps_buf, sizeof(get_gps_buf))<0)
    {
        read(fd2, get_gps_buf, sizeof(get_gps_buf));
        usleep(1000);
        read_time++;
        if(read_time>2)
        {
            gps.latitude=118;
            gps.longitude=32;
            goto exit;
            //break;
        }
    }
    NMEA_GPRMC_Analysis(&gps, get_gps_buf);
    usleep(1000);
    shuju1 = gps.longitude;
    shuju2 = gps.latitude;
    while (shuju1 == 0 || shuju2 == 0)
    {
        NMEA_GPRMC_Analysis(&gps, get_gps_buf);
        shuju1 = gps.longitude;
        shuju2 = gps.latitude;
        read_time2--;
        usleep(1000);
        if (read_time2 == 0)
            break;
    }
    sprintf(buf1,"%ld",gps.longitude);
    sprintf(buf2,"%ld",gps.latitude);
    fwrite(buf1,1,strlen(buf1),f);
    fwrite(buf2,1,strlen(buf2),f);
    printf("longitude is %ld",gps.longitude);
    printf("latitude is %ld",gps.latitude);
    exit:
        fclose(f);
        return gps;
}

int serial_init(void)
{
    fd=openSerial(3);
    fd2=openSerial(1);
    return 0;
}
