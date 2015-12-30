#include <QString>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

#include "dbusdict.h"
#include "mainwindow.h"

QKDBusDict::QKDBusDict(QObject *parent, ArticleNetworkAccessManager *netManager) :
    QObject(parent), m_wnd(NULL)
{
    netMan = netManager;
}

void QKDBusDict::setMainWindow(MainWindow *wnd)
{
    m_wnd = wnd;
}

void QKDBusDict::dataReady()
{
    QString res = QString();
    QNetworkReply* rep = qobject_cast<QNetworkReply *>(sender());
    if (rep!=NULL) {
        res = QString::fromUtf8(rep->readAll());
        rep->deleteLater();
    }

    emit gotWordTranslation(res);
}

void QKDBusDict::findWordTranslation(const QString &text)
{
    QUrl req;
    req.setScheme( "gdlookup" );
    req.setHost( "localhost" );
    QUrlQuery requ;
    requ.addQueryItem( "word", text );
    req.setQuery(requ);
    QNetworkReply* rep = netMan->get(QNetworkRequest(req));
    connect(rep,SIGNAL(finished()),this,SLOT(dataReady()));
}

void QKDBusDict::showDictionaryWindow(const QString &text)
{
    m_wnd->showNormal();
    m_wnd->raise();
    m_wnd->activateWindow();
    m_wnd->setScratchPadText(text);
}
