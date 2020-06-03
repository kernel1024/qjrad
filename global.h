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

class MainWindow;
class QKDBusDict;

namespace CDefaults {
const int maxHButtons = 30;
const int maxKanaHButtons = 15;
const int maxDictionaryResults = 10000;
const int dictSplitterPos = 200;
}

class CGlobal : public QObject
{
    Q_OBJECT
private:
    QStringList dictPaths;

public:
    ZDict::ZDictController* dictManager { nullptr }; // TODO: replace with QScopedPointer
    QKDBusDict* dbusDict { nullptr };

    QFont fontResults;
    QFont fontBtn;
    QFont fontLabels;
    int maxHButtons { CDefaults::maxHButtons };
    int maxKanaHButtons { CDefaults::maxKanaHButtons };
    int maxDictionaryResults { CDefaults::maxDictionaryResults };
    // geometry restore
    bool geomFirstWinPos { true };
    QPoint savedWinPos;
    QSize savedWinSize;
    int savedDictSplitterPos { CDefaults::dictSplitterPos };
    // ---

    explicit CGlobal(QObject *parent = nullptr);
    QStringList getDictPaths();
    void setDictPaths(const QStringList &paths);
    void readSettings();
    void writeSettings(MainWindow *wnd);
    void loadDictionaries();

};

extern CGlobal* cgl;

#ifdef WITH_OCR
extern tesseract::TessBaseAPI* ocr;

tesseract::TessBaseAPI *initializeOCR();
PIX* Image2PIX(QImage& qImage);
#endif

#endif // GLOBAL_H
