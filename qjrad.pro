#-------------------------------------------------
#
# Project created by QtCreator 2012-02-03T20:05:43
#
#-------------------------------------------------

QT       += core gui xml webkit network

TARGET = qjrad
TEMPLATE = app

CONFIG += exceptions \
    rtti \
    stl

LIBS += -lz

SOURCES += main.cpp\
        mainwindow.cpp \
    kdictionary.cpp \
    kanjimodel.cpp \
    settingsdlg.cpp \
    miscutils.cpp \
    goldendict/zipfile.cc \
    goldendict/wstring_qt.cc \
    goldendict/wstring.cc \
    goldendict/utf8.cc \
    goldendict/stardict.cc \
    goldendict/mutex.cc \
    goldendict/langcoder.cc \
    goldendict/folding.cc \
    goldendict/dsl_details.cc \
    goldendict/dsl.cc \
    goldendict/dictzip.c \
    goldendict/indexedzip.cc \
    goldendict/iconv.cc \
    goldendict/htmlescape.cc \
    goldendict/fsencoding.cc \
    goldendict/dictionary.cc \
    goldendict/dictdfiles.cc \
    goldendict/chunkedstorage.cc \
    goldendict/btreeidx.cc \
    goldendict/audiolink.cc \
    goldendict/xdxf2html.cc \
    goldendict/file.cc \
    goldendict/filetype.cc \
    goldendictmgr.cpp \
    global.cpp \
    loadingdlg.cpp \
    goldendict/wordfinder.cc

HEADERS  += mainwindow.h \
    kdictionary.h \
    kanjimodel.h \
    settingsdlg.h \
    miscutils.h \
    goldendict/zipfile.hh \
    goldendict/xdxf2html.hh \
    goldendict/wstring_qt.hh \
    goldendict/wstring.hh \
    goldendict/utf8.hh \
    goldendict/stardict.hh \
    goldendict/sptr.hh \
    goldendict/mutex.hh \
    goldendict/langcoder.hh \
    goldendict/ex.hh \
    goldendict/dsl_details.hh \
    goldendict/dsl.hh \
    goldendict/dictzip.h \
    goldendict/dictionary.hh \
    goldendict/indexedzip.hh \
    goldendict/iconv.hh \
    goldendict/htmlescape.hh \
    goldendict/fsencoding.hh \
    goldendict/folding.hh \
    goldendict/dictdfiles.hh \
    goldendict/chunkedstorage.hh \
    goldendict/btreeidx.hh \
    goldendict/audiolink.hh \
    goldendict/file.hh \
    goldendict/inc_diacritic_folding.hh \
    goldendict/inc_case_folding.hh \
    goldendict/filetype.hh \
    goldendictmgr.h \
    global.h \
    loadingdlg.h \
    goldendict/wordfinder.hh

FORMS    += mainwindow.ui \
    settingsdlg.ui \
    loadingdlg.ui

RESOURCES += \
    qjrad.qrc

SUBDIRS += kanjiconv

OTHER_FILES += \
    kanjiconv/kanjiconv.pro \
    kanjiconv/main.cpp \
    kanjiconv/convertdicts





