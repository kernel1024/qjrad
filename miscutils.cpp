#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>
#include <vector>
#include <QDebug>
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
    QString cnt = content;
    cnt = cnt.replace(QRegularExpression(QSL("\n{3,}")),QSL("\n\n"));
    cnt = cnt.replace(QSL("\n"),QSL("<br />\n"));
    QString res = QSL("<html><head>");
    res.append(QSL("<META HTTP-EQUIV=\"Content-type\" CONTENT=\"text/html; charset=UTF-8;\">"));
    res.append(QSL("<title>%1</title></head>").arg(title));
    res.append(QSL("<body>%1</body></html>").arg(cnt.toHtmlEscaped()));
    return res;
}
