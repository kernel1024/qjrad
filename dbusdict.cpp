#include <QString>
#include <QThread>

#include "dbusdict.h"
#include "mainwindow.h"

QKDBusDict::QKDBusDict(QObject *parent, ZDict::ZDictController *dictManager) :
    QObject(parent), m_wnd(nullptr)
{
    m_dictManager = dictManager;
}

void QKDBusDict::setMainWindow(MainWindow *wnd)
{
    m_wnd = wnd;
}

void QKDBusDict::findWordTranslation(const QString &text)
{
    QThread *th = QThread::create([this,text]{
        QString res = m_dictManager->loadArticle(text);
        Q_EMIT gotWordTranslation(res);
    });
    connect(th,&QThread::finished,th,&QThread::deleteLater);
    th->start();
}

void QKDBusDict::showDictionaryWindow(const QString &text)
{
    m_wnd->showNormal();
    m_wnd->raise();
    m_wnd->activateWindow();
    m_wnd->setScratchPadText(text);
}
