#-------------------------------------------------
#
# Project created by QtCreator 2012-02-03T20:05:43
#
#-------------------------------------------------

QT       += core gui xml widgets network dbus

TARGET = qjrad
TEMPLATE = app

CONFIG += exceptions \
    rtti \
    stl \
    c++11 \
    link_pkgconfig

LIBS += -lz

SOURCES += main.cpp\
        mainwindow.cpp \
    kdictionary.cpp \
    kanjimodel.cpp \
    settingsdlg.cpp \
    miscutils.cpp \
    global.cpp \
    loadingdlg.cpp \
    dbusdict.cpp \
    regiongrabber.cpp \
    xcbtools.cpp

HEADERS  += mainwindow.h \
    kdictionary.h \
    kanjimodel.h \
    settingsdlg.h \
    miscutils.h \
    global.h \
    loadingdlg.h \
    dbusdict.h \
    regiongrabber.h \
    xcbtools.h

FORMS    += mainwindow.ui \
    settingsdlg.ui \
    loadingdlg.ui

RESOURCES += \
    qjrad.qrc

SUBDIRS += kanjiconv

packagesExist(tesseract) {
    CONFIG += use_ocr
    message("Using Tesseract OCR:  YES")
} else {
    message("Using Tesseract OCR:  NO")
}

use_ocr {
    DEFINES += WITH_OCR=1
    QT += x11extras
    QMAKE_CXXFLAGS += -Wno-ignored-qualifiers
    PKGCONFIG += tesseract xcb xcb-xfixes xcb-image
    LIBS += -llept
}

OTHER_FILES += \
    kanjiconv/kanjiconv.pro \
    kanjiconv/main.cpp \
    kanjiconv/convertdicts \
    org.qjrad.dictionary.xml

LIBS += -lgoldendict

DBUS_ADAPTORS = org.qjrad.dictionary.xml
