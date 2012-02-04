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

class QKanjiInfo
{
public:
    QChar kanji;
    int strokes;
    QStringList parts;
    QStringList onReading;
    QStringList kunReading;
    QStringList meaning;
    QKanjiInfo();
    QKanjiInfo(const QChar &aKanji, int aStrokes, const QStringList &aParts,
               const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning);
    QKanjiInfo &operator=(const QKanjiInfo &other);
    bool operator==(const QKanjiInfo &s) const;
    bool operator!=(const QKanjiInfo &s) const;
    bool isEmpty();
};

class QKDictionary : public QObject
{
    Q_OBJECT
public:
    QList<QKRadItem> radicalsLookup;
    QHash<QChar,QKanjiInfo> kanjiInfo;
    QHash<QChar,QStringList> kanjiParts;

    QString errorString;

    explicit QKDictionary(QObject *parent = 0);

    bool loadDictionaries();
    QString sortKanji(const QString src);

signals:

public slots:

};

#endif // KDICTIONARY_H
