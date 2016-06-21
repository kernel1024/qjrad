#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QFont>
#include <QPoint>
#include <QSize>

class MainWindow;
class ArticleNetworkAccessManager;
class CGoldenDictMgr;
class WordFinder;
class QKDBusDict;
class QWebEngineProfile;

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
    QWebEngineProfile* webProfile;

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

#endif // GLOBAL_H
