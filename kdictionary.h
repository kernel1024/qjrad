#ifndef KDICTIONARY_H
#define KDICTIONARY_H

#include <QObject>
#include <QDir>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QDataStream>
#include <QChar>
#include <QString>

using ZKanjiIndex = QHash<unsigned int,qint64>;
using ZKanjiInfoHash = QHash<QChar,int>;

class ZKanjiRadicalItem
{
public:
    QChar radical;
    int strokes { 0 };
    QString jisCode;
    QString kanji;
    ZKanjiRadicalItem() = default;
    ~ZKanjiRadicalItem() = default;
    ZKanjiRadicalItem(const ZKanjiRadicalItem &other) = default;
    ZKanjiRadicalItem(QChar aRadical);
    ZKanjiRadicalItem(QChar aRadical, int aStrokes);
    ZKanjiRadicalItem(QChar aRadical, int aStrokes, const QString &aJisCode, const QString &aKanji);
    ZKanjiRadicalItem &operator=(const ZKanjiRadicalItem &other) = default;
    bool operator==(const ZKanjiRadicalItem &s) const;
    bool operator!=(const ZKanjiRadicalItem &s) const;
    bool operator <(const ZKanjiRadicalItem &ref) const;
    bool operator >(const ZKanjiRadicalItem &ref) const;
};

class ZKanjiInfo
{
    friend QDataStream &operator<<(QDataStream &out, const ZKanjiInfo &obj);
    friend QDataStream &operator>>(QDataStream &in, ZKanjiInfo &obj);
public:
    QChar kanji;
    QStringList parts;
    QStringList onReading;
    QStringList kunReading;
    QStringList meaning;
    ZKanjiInfo() = default;
    ~ZKanjiInfo() = default;
    ZKanjiInfo(const ZKanjiInfo &other) = default;
    ZKanjiInfo(QChar aKanji, const QStringList &aParts,
               const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning);
    ZKanjiInfo &operator=(const ZKanjiInfo &other) = default;
    bool operator==(const ZKanjiInfo &s) const;
    bool operator!=(const ZKanjiInfo &s) const;
    bool isEmpty() const;
};

Q_DECLARE_METATYPE(ZKanjiInfo)

class ZKanjiDictionary : public QObject
{
    Q_OBJECT

private:
    QDir dataPath;

    QString errorString;

    bool parseKanjiDict(QWidget *mainWindow, const QString& xmlDictFileName);
    bool setupDictionaryData(QWidget *mainWindow);
    void deleteDictionaryData();
    bool isDictionaryDataValid();

public:
    QList<ZKanjiRadicalItem> radicalsLookup;
    QHash<QChar,QStringList> kanjiParts;
    ZKanjiInfoHash kanjiStrokes;
    ZKanjiInfoHash kanjiGrade;
    ZKanjiIndex kanjiIndex;
    // TODO: move this indexes to private, also move some kanji search logic from ZMainWindow to separate class

    explicit ZKanjiDictionary(QObject *parent = 0);

    bool loadDictionaries(QWidget *mainWindow);
    QString sortKanji(const QString &src);
    ZKanjiInfo getKanjiInfo(QChar kanji);

    QString getErrorString() const;

public Q_SLOTS:
    void cleanupDictionaries();

};

#endif // KDICTIONARY_H
