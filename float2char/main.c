#include <stdio.h>

char * float2str(float val, int precision, char *buf)
{
    char *cur, *end;

    sprintf(buf, "%.6f", val);
    if (precision < 6) {
        cur = buf + strlen(buf) - 1;
        end = cur - 6 + precision;
        while ((cur > end) && (*cur == '0')) {
            *cur = '\0';
            cur--;
        }
    }

    return buf;
}

int main(int argc, char** argv)
{
    int a,b,c,d;
    /*char buf[128];

    float a=115.1142442;
    sprintf(buf,"%.3g",a);

    printf("%s\n", float2str((float)5, 2, buf));
    printf("%s\n", float2str((float)5.1, 2, buf));
    printf("%s\n", float2str((float)5.12, 2, buf));
    printf("%s\n", float2str((float)5.123, 2, buf));
    printf("%s\n", float2str((float)5.12345678, 2, buf));*/
    char buf[128]={0};
    int i=90;
    sprintf(buf,"%d",i);
    char* s;
    char* p;
    s="90";
    p=buf;
    a=sizeof("90");
    b=sizeof(buf);
    c=sizeof(s);
    d=sizeof(p);

    return 0;
}
