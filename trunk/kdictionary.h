#ifndef KDICTIONARY_H
#define KDICTIONARY_H

#include <QtCore>

class QKRadItem
{
public:
    QChar radical;
    int strokes;
    QString jisCode;
    QString kanji;
    QKRadItem();
    ~QKRadItem();
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
    friend QDataStream &operator<<(QDataStream &out, const QKanjiInfo &obj);
    friend QDataStream &operator>>(QDataStream &in, QKanjiInfo &obj);
public:
    QChar kanji;
    QStringList parts;
    QStringList onReading;
    QStringList kunReading;
    QStringList meaning;
    QKanjiInfo();
    ~QKanjiInfo();
    QKanjiInfo(const QChar &aKanji, const QStringList &aParts,
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
    QHash<QChar,QStringList> kanjiParts;
    QHash<QChar,int> kanjiStrokes;
    QHash<QChar,int> kanjiGrade;

    QString errorString;

    explicit QKDictionary(QObject *parent = 0);

    bool loadDictionaries();
    QString sortKanji(const QString &src);
    QKanjiInfo getKanjiInfo(const QChar &kanji);
private:
    bool loadKanjiDict();
signals:

public slots:

};

#endif // KDICTIONARY_H
