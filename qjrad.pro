QT += core gui widgets dbus xml

TEMPLATE = app

# warn on *any* usage of deprecated APIs
DEFINES += QT_DEPRECATED_WARNINGS
# ... and just fail to compile if APIs deprecated in Qt <= 5.15 are used
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

SOURCES += main.cpp\
    mainwindow.cpp\
    kdictionary.cpp\
    kanjimodel.cpp\
    settingsdlg.cpp\
    global.cpp\
    dbusdict.cpp\
    regiongrabber.cpp\
    xcbtools.cpp

HEADERS += dbusdict.h \
    global.h \
    kanjimodel.h \
    kdictionary.h \
    mainwindow.h \
    qsl.h \
    regiongrabber.h \
    settingsdlg.h \
    xcbtools.h

FORMS += mainwindow.ui\
    settingsdlg.ui

RESOURCES += qjrad.qrc

CONFIG += warn_on link_pkgconfig c++17

LIBS += -lz

packagesExist(tesseract) {
    CONFIG += use_ocr
    message("Using Tesseract OCR:  YES")
} else {
    message("Using Tesseract OCR:  NO")
}

use_ocr {
    DEFINES += WITH_OCR=1
    QMAKE_CXXFLAGS += -Wno-ignored-qualifiers
    PKGCONFIG += xcb xcb-xfixes xcb-image
    PKGCONFIG += tesseract lept
}

DBUS_ADAPTORS = org.qjrad.dictionary.xml

include( zdict/zdict.pri )

OTHER_FILES += \
    org.qjrad.dictionary.xml \
    README.md
