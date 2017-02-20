#ifndef MISCUTILS_H
#define MISCUTILS_H

#include <QObject>
#include <QColor>
#include <QTextBrowser>
#include <QEventLoop>

class CGDTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit CGDTextBrowser(QWidget* parent = Q_NULLPTR);

protected:
    QVariant loadResource(int type, const QUrl &url);

};

class CIOEventLoop : public QEventLoop {
    Q_OBJECT
public:
    CIOEventLoop(QObject* parent = 0);

public slots:
    void finished();
    void timeout();
};

QColor middleColor(const QColor &c1, const QColor &c2, int mul = 50, int div = 100);
QString makeSimpleHtml(const QString &title, const QString &content);

#endif // MISCUTILS_H
