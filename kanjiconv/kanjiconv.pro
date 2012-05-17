#-------------------------------------------------
#
# Project created by QtCreator 2012-05-17T10:13:10
#
#-------------------------------------------------

QT       += core xml

QT       -= gui

TARGET = kanjicnv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

RESOURCES += \
    kanjiconv.qrc


SOURCES += main.cpp\
    ../kdictionary.cpp

HEADERS  += ../kdictionary.h
