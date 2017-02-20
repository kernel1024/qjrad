#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>
#include <vector>
#include <QDebug>
#include <goldendictlib/goldendictmgr.hh>
#include <goldendictlib/dictionary.hh>
#include "global.h"
#include "miscutils.h"

QColor middleColor(const QColor &c1, const QColor &c2, int mul, int div)
{
    QColor res(c1.red()+mul*(c2.red()-c1.red())/div,
               c1.green()+mul*(c2.green()-c1.green())/div,
               c1.blue()+mul*(c2.blue()-c1.blue())/div);
    return res;
}

QString makeSimpleHtml(const QString &title, const QString &content)
{
    QString s = content;
    QString cnt = s.replace(QRegExp("\n{3,}"),"\n\n").replace("\n","<br />\n");
    QString cn="<html><head>";
    cn+="<META HTTP-EQUIV=\"Content-type\" CONTENT=\"text/html; charset=UTF-8;\">";
    cn+="<title>"+title+"</title></head>";
    cn+="<body>"+cnt+"</body></html>";
    return cn;
}

CGDTextBrowser::CGDTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{

}

QVariant CGDTextBrowser::loadResource(int type, const QUrl &url)
{
    if (url.scheme().toLower()=="gdlookup") {
        QByteArray rplb;

        CIOEventLoop ev;
        QString mime;

        QUrlQuery qr(url);
        if ( qr.queryItemValue( "blank" ) == "1" ) {
            rplb = makeSimpleHtml(QString(),QString()).toUtf8();
            return rplb;
        }

        sptr<Dictionary::DataRequest> dr = cgl->netMan->getResource(url,mime);

        connect(dr.get(),SIGNAL(finished()),&ev,SLOT(finished()));
        QTimer::singleShot(15000,&ev,SLOT(timeout()));

        int ret = ev.exec();

        if (ret==1) { // Timeout
            rplb = makeSimpleHtml(tr("Error"),
                                  tr("Dictionary request timeout for query '%1'.")
                                  .arg(url.toString())).toUtf8();

        } else if (dr->isFinished() && dr->dataSize()>0 && ret==0) { // Dictionary success
            std::vector<char> vc = dr->getFullData();
            rplb = QByteArray(reinterpret_cast<const char*>(vc.data()), vc.size());

        } else { // Dictionary error
            rplb = makeSimpleHtml(tr("Error"),
                                  tr("Dictionary request failed for query '%1'.<br/>Error: %2.")
                                  .arg(url.toString(),dr->getErrorString())).toUtf8();
        }
        return rplb;
    }

    return QTextBrowser::loadResource(type,url);
}

CIOEventLoop::CIOEventLoop(QObject *parent)
    : QEventLoop(parent)
{

}

void CIOEventLoop::finished()
{
    exit(0);
}

void CIOEventLoop::timeout()
{
    exit(1);
}
