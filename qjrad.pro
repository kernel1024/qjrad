#-------------------------------------------------
#
# Project created by QtCreator 2012-02-03T20:05:43
#
#-------------------------------------------------

QT       += core gui xml

TARGET = qjrad
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    kdictionary.cpp \
    kanjimodel.cpp \
    settingsdlg.cpp \
    miscutils.cpp

HEADERS  += mainwindow.h \
    kdictionary.h \
    kanjimodel.h \
    settingsdlg.h \
    miscutils.h

FORMS    += mainwindow.ui \
    settingsdlg.ui

RESOURCES += \
    qjrad.qrc

SUBDIRS += kanjiconv

OTHER_FILES += \
    kanjiconv/kanjiconv.pro \
    kanjiconv/main.cpp \
    kanjiconv/convertdicts





