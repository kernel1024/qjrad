#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QFont>
#include <QPoint>
#include <QSize>

#include "zdict/zdictcontroller.h"

#ifdef WITH_OCR
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#if TESSERACT_MAJOR_VERSION>=4
    #define JTESS_API4 1
#endif
#endif

#define zF (ZGlobal::instance())

class ZMainWindow;
class ZKanjiDBusDict;

namespace CDefaults {
const int maxHButtons = 30;
const int maxKanaHButtons = 15;
const int maxDictionaryResults = 10000;
const int dictSplitterPos = 200;
}

class ZGlobal : public QObject
{
    Q_OBJECT
public:
    QPointer<ZDict::ZDictController> dictManager;
    ZKanjiDBusDict* dbusDict { nullptr };

    QFont fontResults() const;
    QFont fontBtn() const;
    QFont fontLabels() const;

    explicit ZGlobal(QObject *parent = nullptr);
    ~ZGlobal() override;
    static ZGlobal* instance();
    void initialize();
    void deferredQuit();

    QStringList getDictPaths();
    void loadDictionaries();
    static QColor middleColor(const QColor &c1, const QColor &c2, int mul = 50, int div = 100);
    static QString makeSimpleHtml(const QString &title, const QString &content);

#ifdef WITH_OCR
    QString ocrGetActiveLanguage();
    QString ocrGetDatapath();
    void initializeOCR();
    QString processImageWithOCR(const QImage& image);
    bool isOCRReady() const;

    static qint64 writeData(QFile* file, const QVariant &data);
    static QVariant readData(QFile* file, const QVariant &defaultValue = QVariant());
private:
    tesseract::TessBaseAPI* m_ocr { nullptr };
    static PIX* Image2PIX(const QImage& qImage);

#endif
};

#endif // GLOBAL_H
