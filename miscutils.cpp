#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>
#include <vector>
#include <QWebEngineUrlRequestJob>
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


CGDSchemeHandler::CGDSchemeHandler(QObject *parent)
    : QWebEngineUrlSchemeHandler(parent)
{

}

void CGDSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    if (request==NULL) return;

    // request->fail doesn't work as expected...

    QByteArray rplb;
    rplb.clear();

    if (request->requestUrl().scheme().toLower()!="gdlookup") {
        rplb = makeSimpleHtml(tr("Error"),
                              tr("Scheme '%1' not supported for 'gdlookup' scheme handler.")
                              .arg(request->requestUrl().scheme())).toUtf8();
        QIODevice *reply = new CMemFile(rplb);
        request->reply("text/html",reply);
        return;
    }

    CIOEventLoop ev;
    QString mime;

    QUrlQuery qr(request->requestUrl());
    if ( qr.queryItemValue( "blank" ) == "1" ) {
        rplb = makeSimpleHtml(QString(),QString()).toUtf8();
        QIODevice *reply = new CMemFile(rplb);
        request->reply("text/html",reply);
        return;
    }

    sptr<Dictionary::DataRequest> dr = cgl->netMan->getResource(request->requestUrl(),mime);

    connect(dr.get(),SIGNAL(finished()),&ev,SLOT(finished()));
    connect(request,SIGNAL(destroyed(QObject*)),&ev,SLOT(objDestroyed(QObject*)));
    QTimer::singleShot(15000,&ev,SLOT(timeout()));

    int ret = ev.exec();

    if (ret==2) { // Request destroyed
        return;

    } else if (ret==1) { // Timeout
        rplb = makeSimpleHtml(tr("Error"),
                              tr("Dictionary request timeout for query '%1'.")
                              .arg(request->requestUrl().toString())).toUtf8();
        QIODevice *reply = new CMemFile(rplb);
        request->reply("text/html",reply);

    } else if (dr->isFinished() && dr->dataSize()>0 && ret==0) { // Dictionary success
        std::vector<char> vc = dr->getFullData();
        QByteArray res = QByteArray(reinterpret_cast<const char*>(vc.data()), vc.size());
        QIODevice *reply = new CMemFile(res);
        request->reply(mime.toLatin1(),reply);

    } else { // Dictionary error
        rplb = makeSimpleHtml(tr("Error"),
                              tr("Dictionary request failed for query '%1'.<br/>Error: %2.")
                              .arg(request->requestUrl().toString(),
                                   dr->getErrorString())).toUtf8();
        QIODevice *reply = new CMemFile(rplb);
        request->reply("text/html",reply);
    }
}

CMemFile::CMemFile(const QByteArray &fileData)
    : data(fileData), origLen(fileData.length())
{
    setOpenMode(QIODevice::ReadOnly);

    QTimer::singleShot(0, this, &QIODevice::readyRead);
    QTimer::singleShot(0, this, &QIODevice::readChannelFinished);
}

qint64 CMemFile::bytesAvailable() const
{
    return data.length() + QIODevice::bytesAvailable();
}

void CMemFile::close()
{
    QIODevice::close();
    deleteLater();
}

qint64 CMemFile::readData(char *buffer, qint64 maxlen)
{
    qint64 len = qMin(qint64(data.length()), maxlen);
    if (len) {
        memcpy(buffer, data.constData(), len);
        data.remove(0, len);
    }
    return len;
}

qint64 CMemFile::writeData(const char *buffer, qint64 maxlen)
{
    Q_UNUSED(buffer);
    Q_UNUSED(maxlen);

    return 0;
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

void CIOEventLoop::objDestroyed(QObject *obj)
{
    Q_UNUSED(obj);

    exit(2);
}
