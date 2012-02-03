#ifndef KDICTIONARY_H
#define KDICTIONARY_H

#include <QtCore>
#include <QtXml>

class QKRadItem
{
public:
    QChar radical;
    int strokes;
    QString jisCode;
    QString kanji;
    QKRadItem();
    QKRadItem(const QChar &aRadical);
    QKRadItem(const QChar &aRadical, int aStrokes);
    QKRadItem(const QChar &aRadical, int aStrokes, const QString &aJisCode, const QString &aKanji);
    QKRadItem &operator=(const QKRadItem &other);
    bool operator==(const QKRadItem &s) const;
    bool operator!=(const QKRadItem &s) const;
    bool operator <(const QKRadItem &ref);
    bool operator >(const QKRadItem &ref);
};

class QKDictionary : public QObject
{
    Q_OBJECT
public:
    QList<QKRadItem> radicalsLookup;
    QDomDocument kanjiDict;
    QHash<QChar,int> kanjiStrokes;


    explicit QKDictionary(QObject *parent = 0);

    bool loadDictionaries();


signals:

public slots:

};

#endif // KDICTIONARY_H
