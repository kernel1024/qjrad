#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QWebEngineProfile>
#include <QDBusConnection>
#include <goldendictlib/goldendictmgr.hh>
#include <goldendictlib/wordfinder.hh>
#include "global.h"
#include "miscutils.h"
#include "mainwindow.h"
#include "dbusdict.h"
#include "dictionary_adaptor.h"

CGlobal* cgl = NULL;

CGlobal::CGlobal(QObject *parent) :
    QObject(parent),
    geomFirstWinPos(false)
{
    dictPaths.clear();

    dictManager = new CGoldenDictMgr(this);
    netMan = new ArticleNetworkAccessManager(this,dictManager);
    wordFinder = new WordFinder(this);

    webProfile = new QWebEngineProfile("qjrad",this);
    webProfile->installUrlSchemeHandler(QByteArray("gdlookup"), new CGDSchemeHandler());

    dbusDict = new QKDBusDict(this,netMan);
    new DictionaryAdaptor(dbusDict);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/",dbusDict);
    dbus.registerService("org.qjrad.dictionary");

}

QString CGlobal::getIndexDir()
{
    QDir result = getHomeDir();

    result.mkpath( "index" );

    if ( !result.cd( "index" ) )
        printf("Cannot use index dir. Home directory write protected.\n");

    return result.path() + QDir::separator();
}

QStringList CGlobal::getDictPaths()
{
    return dictPaths;
}

void CGlobal::setDictPaths(QStringList paths)
{
    dictPaths.clear();
    dictPaths.append(paths);
}

void CGlobal::readSettings()
{
    dictPaths.clear();
    QFont fontResL = QApplication::font("QListView");
    fontResL.setPointSize(14);
    QFont fontBtnL = QApplication::font("QPushButton");
    fontBtnL.setPointSize(14);
    QFont fontBtnLabelL = QApplication::font("QLabel");
    fontBtnLabelL.setPointSize(12);
    fontBtnLabelL.setWeight(QFont::Bold);

    QSettings se("kernel1024","qjrad");
    se.beginGroup("Main");
    fontResults = qvariant_cast<QFont>(se.value("fontResult",fontResL));
    fontBtn = qvariant_cast<QFont>(se.value("fontButton",fontBtnL));
    fontLabels = qvariant_cast<QFont>(se.value("fontLabel",fontBtnLabelL));
    maxHButtons = se.value("maxHButtons",30).toInt();
    maxKanaHButtons = se.value("maxKanaHButtons",15).toInt();
    se.endGroup();
    se.beginGroup("Geometry");
    savedWinPos = se.value("winPos",QPoint(20,20)).toPoint();
    savedWinSize = se.value("winSize",QSize(200,200)).toSize();
    geomFirstWinPos = true;
    bool okconv;
    savedDictSplitterPos = se.value("dictSplitterPos",200).toInt(&okconv);
    if (!okconv) savedDictSplitterPos = 200;
    int dcnt = se.value("dictCount",0).toInt();
    for (int i=0;i<dcnt;i++) {
        QString s = se.value(tr("dict-%1").arg(i),QString()).toString();
        if (!s.isEmpty())
            dictPaths << s;
    }
    se.endGroup();
}

void CGlobal::writeSettings(MainWindow * wnd)
{
    QSettings se("kernel1024","qjrad");
    se.beginGroup("Main");
    se.setValue("fontResult",fontResults);
    se.setValue("fontButton",fontBtn);
    se.setValue("fontLabel",fontLabels);
    se.setValue("maxHButtons",maxHButtons);
    se.setValue("maxKanaHButtons",maxKanaHButtons);
    se.endGroup();
    se.beginGroup("Geometry");
    se.setValue("winPos",wnd->pos());
    se.setValue("winSize",wnd->size());
    QList<int> szs = wnd->getSplittersSize();
    int ssz = 200;
    if (szs.count()>=2) ssz = szs[1];
    se.setValue("splitterPos",ssz);
    szs = wnd->getDictSplittersSize();
    ssz = 200;
    if (szs.count()>=2) ssz = szs[1];
    se.setValue("dictSplitterPos",ssz);
    se.setValue("dictCount",dictPaths.count());
    for (int i=0;i<dictPaths.count();i++)
        se.setValue(tr("dict-%1").arg(i),dictPaths.at(i));
    se.endGroup();
}

void CGlobal::loadDictionaries()
{
    dictManager->loadDictionaries(getDictPaths(),getIndexDir());
}

QDir CGlobal::getHomeDir()
{
    QDir result = QDir::home();

    result.mkpath( ".qjrad" );

    if ( !result.cd( ".qjrad" ) )
        printf("Cannot use home directory. Write protected.\n");

    return result;
}

#ifdef WITH_OCR
PIX* Image2PIX(QImage& qImage) {
  PIX * pixs;
  l_uint32 *lines;

  qImage = qImage.rgbSwapped();
  int width = qImage.width();
  int height = qImage.height();
  int depth = qImage.depth();
  int wpl = qImage.bytesPerLine() / 4;

  pixs = pixCreate(width, height, depth);
  pixSetWpl(pixs, wpl);
  pixSetColormap(pixs, NULL);
  l_uint32 *datas = pixs->data;

  for (int y = 0; y < height; y++) {
    lines = datas + y * wpl;
    QByteArray a((const char*)qImage.scanLine(y), qImage.bytesPerLine());
    for (int j = 0; j < a.size(); j++) {
      *((l_uint8 *)lines + j) = a[j];
    }
  }
  return pixEndianByteSwapNew(pixs);
}

QImage PIX2QImage(PIX *pixImage) {
  int width = pixGetWidth(pixImage);
  int height = pixGetHeight(pixImage);
  int depth = pixGetDepth(pixImage);
  int bytesPerLine = pixGetWpl(pixImage) * 4;
  l_uint32 * s_data = pixGetData(pixEndianByteSwapNew(pixImage));

  QImage::Format format;
  if (depth == 1)
    format = QImage::Format_Mono;
  else if (depth == 8)
    format = QImage::Format_Indexed8;
  else
    format = QImage::Format_RGB32;

  QImage result((uchar*)s_data, width, height, bytesPerLine, format);

  // Handle pallete
  QVector<QRgb> _bwCT;
  _bwCT.append(qRgb(255,255,255));
  _bwCT.append(qRgb(0,0,0));

  QVector<QRgb> _grayscaleCT(256);
  for (int i = 0; i < 256; i++)  {
    _grayscaleCT.append(qRgb(i, i, i));
  }
  if (depth == 1) {
    result.setColorTable(_bwCT);
  }  else if (depth == 8)  {
    result.setColorTable(_grayscaleCT);

  } else {
    result.setColorTable(_grayscaleCT);
  }

  if (result.isNull()) {
    static QImage none(0,0,QImage::Format_Invalid);
    qDebug() << "***Invalid format!!!";
    return none;
  }

  return result.rgbSwapped();
}

tesseract::TessBaseAPI* initializeOCR()
{
    ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL,"jpn")) {
        QMessageBox::critical(NULL,"QManga error",
                              "Could not initialize Tesseract.\n"
                              "Maybe japanese language training data is not installed.");
        return NULL;
    }
    return ocr;
}
#endif
