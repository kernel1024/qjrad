#ifndef MISCUTILS_H
#define MISCUTILS_H

#include <QObject>
#include <QColor>
#include <QTextBrowser>
#include <QEventLoop>

#define QSL QStringLiteral // NOLINT

QColor middleColor(const QColor &c1, const QColor &c2, int mul = 50, int div = 100);
QString makeSimpleHtml(const QString &title, const QString &content);

#endif // MISCUTILS_H
