#include<convert.h>

unsigned char get_modbus1[13] = {0x03,0x04,0x03,0x03,0x16,0x13,0x15,0x00,0x00,0x00,0x00,0x00,0x00};

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

int main()
{
    char andan[3];
    int parameter;
    int result;
    parameter = string_to_int_d(&get_modbus1[3], 2);
    printf("andan is %d.\n",parameter);
    int_to_string(andan, 3, parameter);
    result=string_to_int_d(andan, 4);
    return 0;
}
