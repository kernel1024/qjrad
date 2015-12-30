#-------------------------------------------------
#
# Project created by QtCreator 2012-02-03T20:05:43
#
#-------------------------------------------------

QT       += core gui xml webenginewidgets widgets network dbus

TARGET = qjrad
TEMPLATE = app

CONFIG += exceptions \
    rtti \
    stl \
    c++11

LIBS += -lz

SOURCES += main.cpp\
        mainwindow.cpp \
    kdictionary.cpp \
    kanjimodel.cpp \
    settingsdlg.cpp \
    miscutils.cpp \
    global.cpp \
    loadingdlg.cpp \
    dbusdict.cpp

HEADERS  += mainwindow.h \
    kdictionary.h \
    kanjimodel.h \
    settingsdlg.h \
    miscutils.h \
    global.h \
    loadingdlg.h \
    dbusdict.h

FORMS    += mainwindow.ui \
    settingsdlg.ui \
    loadingdlg.ui

RESOURCES += \
    qjrad.qrc

SUBDIRS += kanjiconv

OTHER_FILES += \
    kanjiconv/kanjiconv.pro \
    kanjiconv/main.cpp \
    kanjiconv/convertdicts \
    org.qjrad.dictionary.xml

LIBS += -lz

include( goldendict/goldendict.pri )

DBUS_ADAPTORS = org.qjrad.dictionary.xml
