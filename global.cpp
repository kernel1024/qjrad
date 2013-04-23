#include "global.h"
#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

CGlobal* cgl = NULL;

CGlobal::CGlobal(QObject *parent) :
    QObject(parent),
    geomFirstWinPos(false)
{
    dictPaths.clear();
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

QDir CGlobal::getHomeDir()
{
    QDir result = QDir::home();

    char const * pathInHome =
        #ifdef Q_OS_WIN32
            "Application Data/GoldenDict"
        #else
            ".qjrad"
        #endif
            ;

    result.mkpath( pathInHome );

    if ( !result.cd( pathInHome ) )
        printf("Cannot use home directory. Write protected.\n");

    return result;
}
