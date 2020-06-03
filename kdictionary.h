#ifndef KDICTIONARY_H
#define KDICTIONARY_H

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QDataStream>
#include <QChar>
#include <QString>

#define QSL QStringLiteral // NOLINT

class QKRadItem
{
public:
    QChar radical;
    int strokes { 0 };
    QString jisCode;
    QString kanji;
    QKRadItem() = default;
    ~QKRadItem() = default;
    QKRadItem(const QKRadItem &other) = default;
    QKRadItem(QChar aRadical);
    QKRadItem(QChar aRadical, int aStrokes);
    QKRadItem(QChar aRadical, int aStrokes, const QString &aJisCode, const QString &aKanji);
    QKRadItem &operator=(const QKRadItem &other) = default;
    bool operator==(const QKRadItem &s) const;
    bool operator!=(const QKRadItem &s) const;
    bool operator <(const QKRadItem &ref) const;
    bool operator >(const QKRadItem &ref) const;
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
    QKanjiInfo() = default;
    ~QKanjiInfo() = default;
    QKanjiInfo(const QKanjiInfo &other) = default;
    QKanjiInfo(QChar aKanji, const QStringList &aParts,
               const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning);
    QKanjiInfo &operator=(const QKanjiInfo &other) = default;
    bool operator==(const QKanjiInfo &s) const;
    bool operator!=(const QKanjiInfo &s) const;
    bool isEmpty() const;
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
    bool loadKanjiDict();
    QString sortKanji(const QString &src);
    QKanjiInfo getKanjiInfo(QChar kanji);

};

#endif // KDICTIONARY_H
