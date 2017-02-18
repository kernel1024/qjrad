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
