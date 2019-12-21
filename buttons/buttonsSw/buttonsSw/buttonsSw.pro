TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
    ../../buttons.c \
    buttonsHall.c \
    task.c

HEADERS += \
    ../../buttons.h \
    buttonsHall.h \
    FreeRTOS.h \
    task.h \
    timers.h
