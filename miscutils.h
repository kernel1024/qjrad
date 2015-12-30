#ifndef MISCUTILS_H
#define MISCUTILS_H

#include <QObject>
#include <QColor>
#include <QIODevice>
#include <QEventLoop>
#include <QWebEngineUrlSchemeHandler>

class CGDSchemeHandler : public QWebEngineUrlSchemeHandler {
    Q_OBJECT
public:
    CGDSchemeHandler(QObject* parent = 0);
    void requestStarted(QWebEngineUrlRequestJob * request);
};

class CMemFile : public QIODevice {
    Q_OBJECT
public:
    CMemFile(const QByteArray &fileData);

    qint64 bytesAvailable() const;
    void close();

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *buffer, qint64 maxlen);

private:
    QByteArray data;
    const qint64 origLen;
};

class CIOEventLoop : public QEventLoop {
    Q_OBJECT
public:
    CIOEventLoop(QObject* parent = 0);

public slots:
    void finished();
    void timeout();
    void objDestroyed(QObject *obj);
};

QColor middleColor(const QColor &c1, const QColor &c2, int mul = 50, int div = 100);
QString makeSimpleHtml(const QString &title, const QString &content);

#endif // MISCUTILS_H
