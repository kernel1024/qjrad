#include "kdictionary.h"
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDir>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QResource>
#include <QDebug>

QKDictionary::QKDictionary(QObject *parent) :
    QObject(parent)
{
    radicalsLookup.clear();
    kanjiParts.clear();
    errorString.clear();
}

bool QKDictionary::loadDictionaries()
{
    radicalsLookup.clear();
    kanjiStrokes.clear();
    kanjiGrade.clear();
    errorString.clear();

    // Load radicals dictionary
    QFile fr(":/data/radkfilex.utf8");
    if (!fr.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji lookup table");
        return false;
    }
    QTextStream sr(&fr);
    sr.setCodec("UTF-8");
    while (!sr.atEnd()) {
        QString s = sr.readLine().trimmed();
        if (s.startsWith('#')) continue; // comment
        else if (s.startsWith('$')) { // new radical
            QStringList sl = s.split(' ');
            QChar krad = sl.at(1).at(0);
            bool okconv;
            int kst = sl.at(2).toInt(&okconv);
            if (!okconv) kst = 0;
            radicalsLookup << QKRadItem(krad,kst,"","");
        } else if (!s.isEmpty() && !radicalsLookup.isEmpty()) {
            radicalsLookup.last().kanji += s;
        }
    }
    fr.close();

    // Load radicals list
    QFile fp(":/data/kradfilex.utf8");
    if (!fp.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji radicals list");
        return false;
    }
    QTextStream sp(&fp);
    sp.setCodec("UTF-8");
    while (!sp.atEnd()) {
        QString s = sp.readLine().trimmed();
        if (s.startsWith('#')) continue; // comment
        if (!s.isEmpty()) {
            QStringList sl = s.split(' ');
            QChar k = sl.takeFirst().at(0);
            sl.takeFirst();
            kanjiParts[k]=sl;
        }
    }
    fp.close();

    return loadKanjiDict();
}

QKDictionary *kdict = NULL;
QMutex kdictmutex;

// Kanji sorting magic
bool kanjiLessThan(const QChar &c1, const QChar &c2)
{
    if (kdict==NULL) return (c1<c2); // in case if kdict is not present - simply compare unicode characters
    if (!kdict->kanjiStrokes.contains(c1) || !kdict->kanjiStrokes.contains(c2)) return (c1<c2); // also here
    // in-depth compare with kdict info
    if (kdict->kanjiStrokes[c1]!=kdict->kanjiStrokes[c2]) // if strokes count differs...
        return (kdict->kanjiStrokes[c1]<kdict->kanjiStrokes[c2]); // compare by strokes count
    else {
        if (kdict->kanjiGrade[c1]!=kdict->kanjiGrade[c2]) // if grade level differs...
            return (kdict->kanjiGrade[c1]!=kdict->kanjiGrade[c2]); // compare by grade
        else
            return (c1<c2); // compare by unicode code inside same-grade/same-strokes groups
    }

}

QString QKDictionary::sortKanji(const QString &src)
{
    kdictmutex.lock();
    kdict = this;
    QString s = src;
    qSort(s.begin(),s.end(),kanjiLessThan);
    kdictmutex.unlock();
    return s;
}

QKanjiInfo QKDictionary::getKanjiInfo(const QChar &kanji)
{
    QKanjiInfo ki = QKanjiInfo();
    QFile f(QString(":/kanjidic/kanji-%1").arg(kanji.unicode()));
    if (!f.open(QIODevice::ReadOnly)) return ki;
    QDataStream fb(&f);
    fb >> ki;
    f.close();
    return ki;
}

bool QKDictionary::loadKanjiDict()
{
    QDir::setSearchPaths("kanjidict",QProcessEnvironment::systemEnvironment().value("PATH").split(':'));
    QDir::addSearchPath("kanjidict",QCoreApplication::applicationDirPath());
    QDir::addSearchPath("kanjidict",QDir::currentPath());
    QDir::addSearchPath("kanjidict","/usr/share/qjrad");
    QDir::addSearchPath("kanjidict","/usr/local/share/qjrad");
    QFileInfo fi("kanjidict:kanjidic.rcc");
    if (!fi.exists()) {
        errorString = tr("Unable to locate main kanjidic.rcc file");
        return false;
    }
    QResource::registerResource("kanjidict:kanjidic.rcc");

    QFile f(":/kanjidic/summary");
    if (!f.open(QIODevice::ReadOnly)) {
        errorString = tr("Unable to load strokes info from dictionary");
        return false;
    }
    QDataStream fb(&f);
    fb >> kanjiStrokes >> kanjiGrade;
    f.close();

    return true;
}

QKRadItem::QKRadItem()
{
    radical = QChar();
    strokes = 0;
    jisCode.clear();
    kanji.clear();
}

QKRadItem::~QKRadItem()
{
    jisCode.clear();
    kanji.clear();
}

QKRadItem::QKRadItem(const QKRadItem &other)
{
    radical = other.radical;
    strokes = other.strokes;
    jisCode = other.jisCode;
    kanji = other.kanji;
}

QKRadItem::QKRadItem(const QChar &aRadical)
{
    radical = aRadical;
    strokes = 0;
    jisCode.clear();
    kanji.clear();
}

QKRadItem::QKRadItem(const QChar &aRadical, int aStrokes)
{
    radical = aRadical;
    strokes = aStrokes;
    jisCode.clear();
    kanji.clear();
}

QKRadItem::QKRadItem(const QChar &aRadical, int aStrokes, const QString &aJisCode, const QString &aKanji)
{
    radical = aRadical;
    strokes = aStrokes;
    jisCode = aJisCode;
    kanji = aKanji;
}

bool QKRadItem::operator ==(const QKRadItem &s) const
{
    return (radical==s.radical);
}

bool QKRadItem::operator !=(const QKRadItem &s) const
{
    return (radical!=s.radical);
}

bool QKRadItem::operator <(const QKRadItem &ref)
{
    if (strokes==ref.strokes)
        return (radical<ref.radical);
    else
        return (strokes<ref.strokes);
}

bool QKRadItem::operator >(const QKRadItem &ref)
{
    if (strokes==ref.strokes)
        return (radical>ref.radical);
    else
        return (strokes>ref.strokes);
}

QKRadItem & QKRadItem::operator =(const QKRadItem &other)
{
    radical = other.radical;
    strokes = other.strokes;
    jisCode = other.jisCode;
    kanji = other.kanji;
    return *this;
}

QKanjiInfo::QKanjiInfo()
{
    kanji = QChar();
    parts.clear();
    onReading.clear();
    kunReading.clear();
    meaning.clear();
}

QKanjiInfo::~QKanjiInfo()
{
    parts.clear();
    onReading.clear();
    kunReading.clear();
    meaning.clear();
}

QKanjiInfo::QKanjiInfo(const QKanjiInfo &other)
{
    kanji = other.kanji;
    parts = other.parts;
    onReading = other.onReading;
    kunReading = other.kunReading;
    meaning = other.meaning;
}

QKanjiInfo::QKanjiInfo(const QChar &aKanji, const QStringList &aParts, const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning)
{
    kanji = aKanji;
    parts = aParts;
    onReading = aOnReading;
    kunReading = aKunReading;
    meaning = aMeaning;
}

QKanjiInfo &QKanjiInfo::operator =(const QKanjiInfo &other)
{
    kanji = other.kanji;
    parts = other.parts;
    onReading = other.onReading;
    kunReading = other.kunReading;
    meaning = other.meaning;
    return *this;
}

bool QKanjiInfo::operator ==(const QKanjiInfo &s) const
{
    return (kanji == s.kanji);
}

bool QKanjiInfo::operator !=(const QKanjiInfo &s) const
{
    return (kanji != s.kanji);
}

bool QKanjiInfo::isEmpty()
{
    return (kanji.isNull());
}



QDataStream &operator <<(QDataStream &out, const QKanjiInfo &obj)
{
    out << obj.kanji << obj.parts << obj.onReading << obj.kunReading << obj.meaning;
    return out;
}


QDataStream &operator >>(QDataStream &in, QKanjiInfo &obj)
{
    in >> obj.kanji >> obj.parts >> obj.onReading >> obj.kunReading >> obj.meaning;
    return in;
}
