#-------------------------------------------------
#
# Project created by QtCreator 2014-03-03T23:42:19
#
#-------------------------------------------------

QT       += core network serialport

QT       -= gui

TARGET = Lexus
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lQtSerialPort

SOURCES += main.cpp \
    bufferize.cpp \
    serialdev.cpp

HEADERS += \
    bufferize.h \
    serialdev.h
