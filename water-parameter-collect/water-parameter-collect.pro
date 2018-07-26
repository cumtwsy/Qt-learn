TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../../ubuntu-iot/iotkit-embedded-master/src/sdk-impl \
               ../../ubuntu-iot/iotkit-embedded-master/src/sdk-impl/imports \
               ../../ubuntu-iot/iotkit-embedded-master/src/sdk-impl/exports



SOURCES += main.c \
    data_collect.c \
    data_send.c \
    mqtt_connect.c \
    ota.c \
    remote_config.c

HEADERS += \
    data_collect.h \
    mqtt_connect.h

