#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>
#include <QtGui>

class MainWindow;

class CGlobal : public QObject
{
    Q_OBJECT
private:
    QStringList dictPaths;
    QDir getHomeDir();
public:
    QFont fontResults, fontBtn, fontLabels;
    int maxHButtons;
    // geometry restore
    bool geomFirstWinPos;
    QPoint savedWinPos;
    QSize savedWinSize;
    int savedSplitterPos;
    int savedDictSplitterPos;
    // ---

    explicit CGlobal(QObject *parent = 0);
    QString getIndexDir();
    QStringList getDictPaths();
    void setDictPaths(QStringList paths);
    void readSettings();
    void writeSettings(MainWindow *wnd);

signals:
    
public slots:
    
};

extern CGlobal* cgl;

#endif // GLOBAL_H
