#ifndef OTA_H
#define OTA_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

#include "iot_import.h"
#include "iot_export.h"
#include "mqtt_connect.h"

int init_ota(void);

#endif // OTA_H

