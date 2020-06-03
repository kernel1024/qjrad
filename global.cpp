#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QDBusConnection>
#include "global.h"
#include "miscutils.h"
#include "mainwindow.h"
#include "dbusdict.h"
#include "dictionary_adaptor.h"

CGlobal* cgl = nullptr; // TODO: upgrade to singleton

namespace CDefaults {
const int kanjiFontSize = 14;
const int labelFontSize = 12;
const QSize windowSize = QSize(200,200);
const QPoint windowPos = QPoint(20,20);
}

CGlobal::CGlobal(QObject *parent) :
    QObject(parent),
    geomFirstWinPos(false)
{
    dictManager = new ZDict::ZDictController(this);

    dbusDict = new QKDBusDict(this,dictManager);
    new DictionaryAdaptor(dbusDict);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QSL("/"),dbusDict);
    dbus.registerService(QSL("org.qjrad.dictionary"));
}

QStringList CGlobal::getDictPaths()
{
    return dictPaths;
}

void CGlobal::setDictPaths(const QStringList &paths)
{
    dictPaths = paths;
}

void CGlobal::readSettings()
{
    dictPaths.clear();
    QFont fontResL = QApplication::font("QListView");
    fontResL.setPointSize(CDefaults::kanjiFontSize);
    QFont fontBtnL = QApplication::font("QPushButton");
    fontBtnL.setPointSize(CDefaults::kanjiFontSize);
    QFont fontBtnLabelL = QApplication::font("QLabel");
    fontBtnLabelL.setPointSize(CDefaults::labelFontSize);
    fontBtnLabelL.setWeight(QFont::Bold);

    QSettings se(QSL("kernel1024"),QSL("qjrad"));
    se.beginGroup(QSL("Main"));
    fontResults = qvariant_cast<QFont>(se.value(QSL("fontResult"),fontResL));
    fontBtn = qvariant_cast<QFont>(se.value(QSL("fontButton"),fontBtnL));
    fontLabels = qvariant_cast<QFont>(se.value(QSL("fontLabel"),fontBtnLabelL));
    maxHButtons = se.value(QSL("maxHButtons"),CDefaults::maxHButtons).toInt();
    maxKanaHButtons = se.value(QSL("maxKanaHButtons"),CDefaults::maxKanaHButtons).toInt();
    maxDictionaryResults = se.value(QSL("maxDictionaryResults"),CDefaults::maxDictionaryResults).toInt();
    se.endGroup();
    se.beginGroup(QSL("Geometry"));
    savedWinPos = se.value(QSL("winPos"),CDefaults::windowPos).toPoint();
    savedWinSize = se.value(QSL("winSize"),CDefaults::windowSize).toSize();
    geomFirstWinPos = true;
    bool okconv = false;
    savedDictSplitterPos = se.value(QSL("dictSplitterPos"),CDefaults::dictSplitterPos).toInt(&okconv);
    if (!okconv) savedDictSplitterPos = CDefaults::dictSplitterPos;
    int dcnt = se.value(QSL("dictCount"),0).toInt();
    for (int i=0;i<dcnt;i++) {
        QString s = se.value(QSL("dict-%1").arg(i),QString()).toString();
        if (!s.isEmpty())
            dictPaths << s;
    }
    se.endGroup();
}

void CGlobal::writeSettings(MainWindow * wnd)
{
    QSettings se(QSL("kernel1024"),QSL("qjrad"));
    se.beginGroup(QSL("Main"));
    se.setValue(QSL("fontResult"),fontResults);
    se.setValue(QSL("fontButton"),fontBtn);
    se.setValue(QSL("fontLabel"),fontLabels);
    se.setValue(QSL("maxHButtons"),maxHButtons);
    se.setValue(QSL("maxKanaHButtons"),maxKanaHButtons);
    se.setValue(QSL("maxDictionaryResults"),maxDictionaryResults);
    se.endGroup();
    se.beginGroup(QSL("Geometry"));
    se.setValue(QSL("winPos"),wnd->pos());
    se.setValue(QSL("winSize"),wnd->size());
    QList<int> szs = wnd->getSplittersSize();
    int ssz = CDefaults::dictSplitterPos;
    if (szs.count()>=2) ssz = szs[1];
    se.setValue(QSL("splitterPos"),ssz);
    szs = wnd->getDictSplittersSize();
    ssz = CDefaults::dictSplitterPos;
    if (szs.count()>=2) ssz = szs[1];
    se.setValue(QSL("dictSplitterPos"),ssz);
    se.setValue(QSL("dictCount"),dictPaths.count());
    for (int i=0;i<dictPaths.count();i++)
        se.setValue(QSL("dict-%1").arg(i),dictPaths.at(i));
    se.endGroup();
}

void CGlobal::loadDictionaries()
{
    dictManager->loadDictionaries(getDictPaths());
}

#ifdef WITH_OCR
PIX* Image2PIX(QImage& qImage) {
    PIX * pixs = nullptr;
    l_uint32 *lines = nullptr;

    qImage = qImage.rgbSwapped();
    int width = qImage.width();
    int height = qImage.height();
    int depth = qImage.depth();
    int wpl = qImage.bytesPerLine() / 4;

    pixs = pixCreate(width, height, depth);
    pixSetWpl(pixs, wpl);
    pixSetColormap(pixs, nullptr);
    l_uint32 *datas = pixs->data;

    for (int y = 0; y < height; y++) {
        lines = datas + y * wpl;
        QByteArray a(reinterpret_cast<const char*>(qImage.scanLine(y)), qImage.bytesPerLine());
        for (int j = 0; j < a.size(); j++) {
            *((l_uint8 *)lines + j) = a[j];
        }
    }
    return pixEndianByteSwapNew(pixs);
}

tesseract::TessBaseAPI* initializeOCR()
{
    ocr = new tesseract::TessBaseAPI(); // TODO: take tesseract initialization from QManga
    if (ocr->Init(nullptr,"jpn")) {
        QMessageBox::critical(nullptr,QSL("QJRad error"),
                              QSL("Could not initialize Tesseract.\n"
                                  "Maybe japanese language training data is not installed."));
        return nullptr;
    }
    return ocr;
}
#endif
