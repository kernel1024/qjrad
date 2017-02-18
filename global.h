#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QFont>
#include <QPoint>
#include <QSize>

#ifdef WITH_OCR
#include <baseapi.h>
#include <leptonica/allheaders.h>
#endif

class MainWindow;
class ArticleNetworkAccessManager;
class CGoldenDictMgr;
class WordFinder;
class QKDBusDict;

class CGlobal : public QObject
{
    Q_OBJECT
private:
    QStringList dictPaths;
    QDir getHomeDir();
public:
    ArticleNetworkAccessManager * netMan;
    CGoldenDictMgr * dictManager;
    WordFinder * wordFinder;
    QKDBusDict* dbusDict;

    QFont fontResults, fontBtn, fontLabels;
    int maxHButtons, maxKanaHButtons;
    // geometry restore
    bool geomFirstWinPos;
    QPoint savedWinPos;
    QSize savedWinSize;
    int savedDictSplitterPos;
    // ---

    explicit CGlobal(QObject *parent = 0);
    QString getIndexDir();
    QStringList getDictPaths();
    void setDictPaths(QStringList paths);
    void readSettings();
    void writeSettings(MainWindow *wnd);
    void loadDictionaries();

signals:
    
public slots:
    
};

extern CGlobal* cgl;

#ifdef WITH_OCR
extern tesseract::TessBaseAPI* ocr;

tesseract::TessBaseAPI *initializeOCR();
PIX* Image2PIX(QImage& qImage);
QImage PIX2QImage(PIX *pixImage);
#endif

#endif // GLOBAL_H
