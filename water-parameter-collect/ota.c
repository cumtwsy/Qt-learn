/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include"ota.h"

#define PRODUCT_KEY             "a1iwqDlN76y"
#define DEVICE_NAME             "hemu001"
#define DEVICE_SECRET           "P9fYXUx9sTEqKfnjnxyuyNVf9uWwjEiF"

extern void* pclient;

char g_product_key[PRODUCT_KEY_LEN + 1];
char g_product_secret[PRODUCT_SECRET_LEN + 1];
char g_device_name[DEVICE_NAME_LEN + 1];
char g_device_secret[DEVICE_SECRET_LEN + 1];

/* These are pre-defined topics */
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA              "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define OTA_MQTT_MSGLEN         (2048)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static int      user_argc;
static char   **user_argv;

void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

int mqtt_client(void)
{
    #define OTA_BUF_LEN        (5000)

    int rc = 0, ota_over = 0;
    void *h_ota = NULL;
    char *msg_buf = NULL, *msg_readbuf = NULL;
    FILE *fp;
    char buf_ota[OTA_BUF_LEN];

    if (NULL == (fp = fopen("ota.bin", "wb+"))) {
        EXAMPLE_TRACE("open file failed");
        goto do_exit;
    }
    if(pclient==NULL)
    {
        usleep(100);
    }
    h_ota = IOT_OTA_Init(PRODUCT_KEY, DEVICE_NAME, pclient);
    if (NULL == h_ota) {
        rc = -1;
        EXAMPLE_TRACE("initialize OTA failed");
        goto do_exit;
    }

    if (0 != IOT_OTA_ReportVersion(h_ota, "iotx_ver_1.0.0")) {
        rc = -1;
        EXAMPLE_TRACE("report OTA version failed");
        goto do_exit;
    }

    HAL_SleepMs(1000);

    do {
        uint32_t firmware_valid;

        EXAMPLE_TRACE("wait ota upgrade command....");

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        if (IOT_OTA_IsFetching(h_ota)) {
            uint32_t last_percent = 0, percent = 0;
            char version[128], md5sum[33];
            uint32_t len, size_downloaded, size_file;
            do {

                len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    if (1 != fwrite(buf_ota, len, 1, fp)) {
                        EXAMPLE_TRACE("write data to file failed");
                        rc = -1;
                        break;
                    }
                } else {
                    IOT_OTA_ReportProgress(h_ota, IOT_OTAP_FETCH_FAILED, NULL);
                    EXAMPLE_TRACE("ota fetch fail");
                }

                /* get OTA information */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &size_file, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_MD5SUM, md5sum, 33);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, version, 128);

                last_percent = percent;
                percent = (size_downloaded * 100) / size_file;
                if (percent - last_percent > 0) {
                    IOT_OTA_ReportProgress(h_ota, percent, NULL);
                    IOT_OTA_ReportProgress(h_ota, percent, "hello");
                }
                IOT_MQTT_Yield(pclient, 100);
            } while (!IOT_OTA_IsFetchFinish(h_ota));

            IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
            if (0 == firmware_valid) {
                EXAMPLE_TRACE("The firmware is invalid");
            } else {
                EXAMPLE_TRACE("The firmware is valid");
            }

            ota_over = 1;
        }
        HAL_SleepMs(2000);
    } while (!ota_over);

    HAL_SleepMs(200);


do_exit:

    if (NULL != h_ota) {
        IOT_OTA_Deinit(h_ota);
    }

    if (NULL != pclient) {
        IOT_MQTT_Destroy(&pclient);
    }

    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    if (NULL != fp) {
        fclose(fp);
    }

    return rc;
}


int init_ota(void)
{
    int ret;
    pthread_t thread_show;
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);
    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);
    ret = pthread_create(&thread_show, PTHREAD_CREATE_JOINABLE, (void *)mqtt_client, NULL);
    if (ret)
    {
        printf("create thread faile \n");
        return -1;
    }
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!");
    return 0;

}




/*
 * int main(int argc, char **argv)
{
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = argc;
    user_argv = argv;


    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);

    mqtt_client();

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!");
    return 0;
}*/


