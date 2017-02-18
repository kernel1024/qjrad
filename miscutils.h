#ifndef MISCUTILS_H
#define MISCUTILS_H

#include <QObject>
#include <QColor>

QColor middleColor(const QColor &c1, const QColor &c2, int mul = 50, int div = 100);
QString makeSimpleHtml(const QString &title, const QString &content);

#endif // MISCUTILS_H
