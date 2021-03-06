#-------------------------------------------------
#
# Project created by QtCreator 2014-03-03T23:42:19
#
#-------------------------------------------------

QT       += core network serialport

QT       -= gui

TARGET = Slave
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lQtSerialPort

SOURCES += main.cpp \
    bufferize.cpp \
    serialdev.cpp \
    tcpclient.cpp \
    tcpserver.cpp \
    udpserver.cpp \
    utils.cpp \
    powermanager.cpp

HEADERS += \
    bufferize.h \
    serialdev.h \
    tcpclient.h \
    tcpserver.h \
    udpserver.h \
    utils.h \
    powermanager.h
