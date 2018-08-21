#include <stdio.h>
#include "data_collect.h"
#include "ota.h"
#include "mqtt_connect.h"

int main(void)
{
    init_ota();
    mqtt_init();
    while(1)
    {
        sleep(1);
    }

}

