#include <QString>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#include "dbusdict.h"
#include "mainwindow.h"

QKDBusDict::QKDBusDict(QObject *parent, ArticleNetworkAccessManager *netManager) :
    QObject(parent)
{
    netMan = netManager;
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
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    req.addQueryItem( "word", text );
#else
    QUrlQuery requ;
    requ.addQueryItem( "word", text );
    req.setQuery(requ);
#endif
    QNetworkReply* rep = netMan->get(QNetworkRequest(req));
    connect(rep,SIGNAL(finished()),this,SLOT(dataReady()));
}

void QKDBusDict::showDictionaryWindow(const QString &text)
{
    MainWindow* w = qobject_cast<MainWindow *>(parent());
    if (w==NULL) return;
    w->showNormal();
    w->raise();
    w->activateWindow();
    w->setScratchPadText(text);
}
