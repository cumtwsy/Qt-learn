#include"data_collect.h"

int main(void)
{
    DATA data1;
    nmea_msg data2;
    serial_init();
    while(1)
    {
       data1=data_collect2();
       data2=get_gps();
       printf("%d",data2.latitude);
       printf("%d",data2.longitude);
       sleep(1);
    }
    return 0;
}
