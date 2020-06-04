#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QDBusConnection>
#include "global.h"
#include "qsl.h"
#include "mainwindow.h"
#include "dbusdict.h"
#include "dictionary_adaptor.h"

namespace CDefaults {
const int kanjiFontSize = 14;
const int labelFontSize = 12;
}

ZGlobal::ZGlobal(QObject *parent) :
    QObject(parent)
{
    dictManager = new ZDict::ZDictController(this);

    dbusDict = new ZKanjiDBusDict(this,dictManager);
    new DictionaryAdaptor(dbusDict);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QSL("/"),dbusDict);
    dbus.registerService(QSL("org.qjrad.dictionary"));
}

ZGlobal::~ZGlobal() = default;

ZGlobal *ZGlobal::instance()
{
    static QPointer<ZGlobal> inst;
    static QAtomicInteger<bool> initializedOnce(false);

    if (inst.isNull()) {
        if (initializedOnce.testAndSetAcquire(false,true)) {
            inst = new ZGlobal(QApplication::instance());
            return inst.data();
        }

        qCritical() << "Accessing to zF after destruction!!!";
        return nullptr;
    }

    return inst.data();
}

void ZGlobal::initialize()
{
#if TESSERACT_MAJOR_VERSION>=4
    setlocale (LC_ALL, "C");
    setlocale (LC_CTYPE, "C");
#endif
    setlocale (LC_NUMERIC, "C");

#ifdef WITH_OCR
    initializeOCR();
#endif

    qRegisterMetaType<ZKanjiRadicalItem>("QKRadItem");
    qRegisterMetaType<ZKanjiInfo>("QKanjiInfo");

    QGuiApplication::setApplicationDisplayName(QSL("QJRad - Kanji Lookup Tool"));
    QCoreApplication::setOrganizationName(QSL("kernel1024"));
    QCoreApplication::setApplicationName(QSL("qjrad"));
}

QStringList ZGlobal::getDictPaths()
{
    QStringList res;

    QSettings stg;
    stg.beginGroup(QSL("Dictionaries"));
    const QStringList dicts = stg.childKeys();
    res.reserve(dicts.count());
    for (const auto &key : dicts) {
        QString path = stg.value(key,QString()).toString();
        if (!path.isEmpty())
            res.append(path);
    }
    stg.endGroup();
    return res;
}

QFont ZGlobal::fontResults() const
{
    QSettings stg;
    stg.beginGroup(QSL("Main"));
    QFont fontResL = QApplication::font("QListView");
    fontResL.setPointSize(CDefaults::kanjiFontSize);
    auto fontResults = qvariant_cast<QFont>(stg.value(QSL("fontResult"),fontResL));
    stg.endGroup();

    return fontResults;
}

QFont ZGlobal::fontBtn() const
{
    QSettings stg;
    stg.beginGroup(QSL("Main"));
    QFont fontBtnL = QApplication::font("QPushButton");
    fontBtnL.setPointSize(CDefaults::kanjiFontSize);
    auto fontBtn = qvariant_cast<QFont>(stg.value(QSL("fontButton"),fontBtnL));
    stg.endGroup();

    return fontBtn;
}

QFont ZGlobal::fontLabels() const
{
    QSettings stg;
    stg.beginGroup(QSL("Main"));
    QFont fontBtnLabelL = QApplication::font("QLabel");
    fontBtnLabelL.setPointSize(CDefaults::labelFontSize);
    fontBtnLabelL.setWeight(QFont::Bold);
    auto fontLabels = qvariant_cast<QFont>(stg.value(QSL("fontLabel"),fontBtnLabelL));
    stg.endGroup();

    return fontLabels;
}

void ZGlobal::loadDictionaries()
{
    dictManager->loadDictionaries(getDictPaths());
}

QColor ZGlobal::middleColor(const QColor &c1, const QColor &c2, int mul, int div)
{
    QColor res(c1.red()+mul*(c2.red()-c1.red())/div,
               c1.green()+mul*(c2.green()-c1.green())/div,
               c1.blue()+mul*(c2.blue()-c1.blue())/div);
    return res;
}

QString ZGlobal::makeSimpleHtml(const QString &title, const QString &content)
{
    QString cnt = content;
    cnt = cnt.replace(QRegularExpression(QSL("\n{3,}")),QSL("\n\n"));
    cnt = cnt.replace(QSL("\n"),QSL("<br />\n"));
    QString res = QSL("<html><head>");
    res.append(QSL("<META HTTP-EQUIV=\"Content-type\" CONTENT=\"text/html; charset=UTF-8;\">"));
    res.append(QSL("<title>%1</title></head>").arg(title));
    res.append(QSL("<body>%1</body></html>").arg(cnt.toHtmlEscaped()));
    return res;
}

#ifdef WITH_OCR

QString ZGlobal::processImageWithOCR(const QImage &image)
{
    m_ocr->SetImage(Image2PIX(image));
    char* rtext = m_ocr->GetUTF8Text();
    QString s = QString::fromUtf8(rtext);
    delete[] rtext;
    return s;
}

bool ZGlobal::isOCRReady() const
{
    return (m_ocr!=nullptr);
}

QString ZGlobal::ocrGetActiveLanguage()
{
    QSettings settings;
    settings.beginGroup(QSL("OCR"));
    QString res = settings.value(QSL("activeLanguage"),QSL("jpn")).toString();
    settings.endGroup();
    return res;
}

QString ZGlobal::ocrGetDatapath()
{
    QSettings settings;
    settings.beginGroup(QSL("OCR"));
    QString res = settings.value(QSL("datapath"),
                                 QSL("/usr/share/tessdata/")).toString();
    settings.endGroup();
    return res;
}

PIX* ZGlobal::Image2PIX(const QImage &qImage) {
    PIX * pixs = nullptr;
    l_uint32 *lines = nullptr;

    QImage img = qImage.rgbSwapped();
    int width = img.width();
    int height = img.height();
    int depth = img.depth();
    int wpl = img.bytesPerLine() / 4;

    pixs = pixCreate(width, height, depth);
    pixSetWpl(pixs, wpl);
    pixSetColormap(pixs, nullptr);
    l_uint32 *datas = pixs->data;

    for (int y = 0; y < height; y++) {
        lines = datas + y * wpl;
        memcpy(lines,img.scanLine(y),static_cast<uint>(img.bytesPerLine()));
    }
    return pixEndianByteSwapNew(pixs);
}

void ZGlobal::initializeOCR()
{
    m_ocr = new tesseract::TessBaseAPI();
    QByteArray tesspath = ocrGetDatapath().toUtf8();

    QString lang = ocrGetActiveLanguage();
    if (lang.isEmpty() || (m_ocr->Init(tesspath.constData(),lang.toUtf8().constData())!=0)) {
        qCritical() << "Could not initialize Tesseract. "
                       "Maybe language training data is not installed.";
    }
}

#endif
