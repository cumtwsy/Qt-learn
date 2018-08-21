#include <stdio.h>
#include <stdlib.h>
int main(){
    FILE *f;//输入文件
    long len;//文件长度
    int time_lode;
    char *content;//文件内容
    char *result=malloc(3);
    f=fopen("/opt/alinkota.bin","rb");
    fseek(f,0,SEEK_END);
    len=ftell(f);
    fseek(f,0,SEEK_SET);
    content=(char*)malloc(len+1);
    fread(content,1,len,f);
    for(int i=0;i<len;i++)
    {
        if(content[i]==':')
        {
            printf("find the symbol.\n");
            for(int j=i;j<len;j++)
            {
                if(content[j]>='0'&&content[j]<='9')
                {
                    result[j-i]=content[j];
                    continue;
                }
                i++;
            }
            time_lode=atoi(result);
            break;
        }
        else
            continue;
    }
    fclose(f);
    free(content);
    free(result);
    return time_lode;
}

