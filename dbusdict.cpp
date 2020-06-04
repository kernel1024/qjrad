#include <QString>
#include <QThread>

#include "dbusdict.h"
#include "mainwindow.h"

ZKanjiDBusDict::ZKanjiDBusDict(QObject *parent, ZDict::ZDictController *dictManager) :
    QObject(parent), m_wnd(nullptr)
{
    m_dictManager = dictManager;
}

void ZKanjiDBusDict::setMainWindow(ZMainWindow *wnd)
{
    m_wnd = wnd;
}

void ZKanjiDBusDict::findWordTranslation(const QString &text)
{
    QThread *th = QThread::create([this,text]{
        QString res = m_dictManager->loadArticle(text);
        Q_EMIT gotWordTranslation(res);
    });
    connect(th,&QThread::finished,th,&QThread::deleteLater);
    th->start();
}

void ZKanjiDBusDict::showDictionaryWindow(const QString &text)
{
    m_wnd->showNormal();
    m_wnd->raise();
    m_wnd->activateWindow();
    m_wnd->setScratchPadText(text);
}
