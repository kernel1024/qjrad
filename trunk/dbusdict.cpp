#include <QString>
#include <QWebPage>
#include <QWebFrame>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

#include "dbusdict.h"

QKDBusDict::QKDBusDict(QObject *parent) :
    QObject(parent)
{
    hdlg = new QKDBusTranDlg();
    connect(hdlg->webView,SIGNAL(loadFinished(bool)),this,SLOT(loadFinished(bool)));
}

QKDBusDict::~QKDBusDict()
{
    hdlg->deleteLater();
}


void QKDBusDict::loadFinished(bool ok)
{
    QString res = QString();
    if (ok && hdlg->webView->page()!=NULL)
        if (hdlg->webView->page()->mainFrame()!=NULL)
            res = hdlg->webView->page()->mainFrame()->toHtml();

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
    hdlg->webView->load(req);
}
