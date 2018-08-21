#include "data-collect.h"

DATA3 data1;

char* delete_tail(char* data)
{
    int i=0;
    int j=strlen(data);
    //char buf[strlen(data)+1];
    char* buf2=malloc((j+1)*sizeof(char*));
    //char* buf;
    for(i=0;i<j;i++)
    {
        buf2[i]=data[i];
    }
    buf2[strlen(data)+1]='\0';
    //free(buf);
    return buf2;
}

void data_test()
{
    char buf[128]={0};
    char* buf_load;
    float a=24.766;
    sprintf(buf,"%g",a);
    buf_load=delete_tail(buf);
    data1.air_temp=buf_load;
    //free(buf_load);
}
