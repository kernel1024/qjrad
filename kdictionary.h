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

class ZKanjiRadicalItem {
public:
    int strokes { 0 };
    QString kanji;
    ZKanjiRadicalItem() = default;
    ~ZKanjiRadicalItem() = default;
    ZKanjiRadicalItem(int aStrokes, const QString& aKanji);
    ZKanjiRadicalItem &operator=(const ZKanjiRadicalItem &other) = default;
};

class ZKanjiInfo
{
    friend QDataStream &operator<<(QDataStream &out, const ZKanjiInfo &obj);
    friend QDataStream &operator>>(QDataStream &in, ZKanjiInfo &obj);
public:
    QChar kanji;
    QStringList onReading;
    QStringList kunReading;
    QStringList meaning;
    ZKanjiInfo() = default;
    ~ZKanjiInfo() = default;
    ZKanjiInfo(const ZKanjiInfo &other) = default;
    ZKanjiInfo(QChar aKanji, const QStringList &aOnReading, const QStringList &aKunReading,
               const QStringList &aMeaning);
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
    QList<QPair<QChar,int> > m_radicalsList;
    QHash<QChar,ZKanjiRadicalItem> m_radicalsLookup;
    QHash<QChar,QString> m_kanjiParts;
    ZKanjiInfoHash m_kanjiStrokes;
    ZKanjiInfoHash m_kanjiGrade;
    ZKanjiIndex m_kanjiIndex;

    QDir m_dataPath;
    QString m_errorString;

    bool parseKanjiDict(QWidget *mainWindow, const QString& xmlDictFileName);
    bool setupDictionaryData(QWidget *mainWindow);
    void deleteDictionaryData();
    bool isDictionaryDataValid();

public:
    explicit ZKanjiDictionary(QObject *parent = 0);

    bool loadDictionaries(QWidget *mainWindow);
    QString getErrorString() const;

    QString sortKanji(const QString &src);
    ZKanjiInfo getKanjiInfo(QChar kanji);
    int getKanjiGrade(const QChar &kanji) const;
    int getKanjiStrokes(const QChar &kanji) const;
    const QList<QPair<QChar,int> > &getAllRadicals() const;
    ZKanjiRadicalItem getRadicalInfo(const QChar &radical) const;
    QString lookupRadicals(const QString &radicals) const;
    QString getKanjiParts(const QChar &kanji) const;

public Q_SLOTS:
    void cleanupDictionaries();

};

#endif // KDICTIONARY_H
