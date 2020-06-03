#include "kdictionary.h"
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDir>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QResource>
#include <QDebug>
#include <algorithm>

QKDictionary::QKDictionary(QObject *parent) :
    QObject(parent)
{
}

bool QKDictionary::loadDictionaries()
{
    radicalsLookup.clear();
    kanjiStrokes.clear();
    kanjiGrade.clear();
    errorString.clear();

    // Load radicals dictionary
    QFile fr(QSL(":/data/radkfilex.utf8"));
    if (!fr.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji lookup table");
        return false;
    }
    QTextStream sr(&fr);
    sr.setCodec("UTF-8");
    while (!sr.atEnd()) {
        QString s = sr.readLine().trimmed();
        if (s.startsWith('#')) continue; // comment
        if (s.startsWith('$')) { // new radical
            QStringList sl = s.split(' ');
            QChar krad = sl.at(1).at(0);
            bool okconv = false;
            int kst = sl.at(2).toInt(&okconv);
            if (!okconv) kst = 0;
            radicalsLookup << QKRadItem(krad,kst,QString(),QString());
        } else if (!s.isEmpty() && !radicalsLookup.isEmpty()) {
            radicalsLookup.last().kanji += s;
        }
    }
    fr.close();

    // Load radicals list
    QFile fp(QSL(":/data/kradfilex.utf8"));
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

QKDictionary *kdict = nullptr;
QMutex kdictmutex;

// Kanji sorting magic
bool kanjiLessThan(QChar c1, QChar c2)
{
    if (kdict==nullptr) // in case if kdict is not present - simply compare unicode characters
        return (c1<c2);

    if (!kdict->kanjiStrokes.contains(c1) || !kdict->kanjiStrokes.contains(c2)) // also here
        return (c1<c2);

    // in-depth compare with kdict info
    if (kdict->kanjiStrokes.value(c1)!=kdict->kanjiStrokes.value(c2)) // if strokes count differs...
        return (kdict->kanjiStrokes.value(c1)<kdict->kanjiStrokes.value(c2)); // compare by strokes count

    if (kdict->kanjiGrade.value(c1)!=kdict->kanjiGrade.value(c2)) // if grade level differs...
        return (kdict->kanjiGrade.value(c1)!=kdict->kanjiGrade.value(c2)); // compare by grade

    return (c1<c2); // compare by unicode code inside same-grade/same-strokes groups
}

QString QKDictionary::sortKanji(const QString &src)
{
    kdictmutex.lock();
    kdict = this;
    QString s = src;
    std::sort(s.begin(),s.end(),kanjiLessThan);
    kdictmutex.unlock();
    return s;
}

QKanjiInfo QKDictionary::getKanjiInfo(QChar kanji)
{
    QKanjiInfo ki = QKanjiInfo();
    QFile f(QSL(":/kanjidic/kanji-%1").arg(kanji.unicode()));
    if (!f.open(QIODevice::ReadOnly)) return ki;
    QDataStream fb(&f);
    fb >> ki;
    f.close();
    return ki;
}

bool QKDictionary::loadKanjiDict()
{
    QDir::setSearchPaths(QSL("kanjidict"),QProcessEnvironment::systemEnvironment().value(QSL("PATH")).split(':'));
    QDir::addSearchPath(QSL("kanjidict"),QCoreApplication::applicationDirPath());
    QDir::addSearchPath(QSL("kanjidict"),QDir::currentPath());
    QDir::addSearchPath(QSL("kanjidict"),QSL("/usr/share/qjrad"));
    QDir::addSearchPath(QSL("kanjidict"),QSL("/usr/local/share/qjrad"));
    QFileInfo fi(QSL("kanjidict:kanjidic.rcc"));
    if (!fi.exists()) {
        errorString = tr("Unable to locate main kanjidic.rcc file");
        return false;
    }
    QResource::registerResource(QSL("kanjidict:kanjidic.rcc"));

    QFile f(QSL(":/kanjidic/summary"));
    if (!f.open(QIODevice::ReadOnly)) {
        errorString = tr("Unable to load strokes info from dictionary");
        return false;
    }
    QDataStream fb(&f);
    fb >> kanjiStrokes >> kanjiGrade;
    f.close();

    return true;
}

QKRadItem::QKRadItem(QChar aRadical)
{
    radical = aRadical;
    strokes = 0;
    jisCode.clear();
    kanji.clear();
}

QKRadItem::QKRadItem(QChar aRadical, int aStrokes)
{
    radical = aRadical;
    strokes = aStrokes;
    jisCode.clear();
    kanji.clear();
}

QKRadItem::QKRadItem(QChar aRadical, int aStrokes, const QString &aJisCode, const QString &aKanji)
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

bool QKRadItem::operator <(const QKRadItem &ref) const
{
    if (strokes==ref.strokes)
        return (radical<ref.radical);

    return (strokes<ref.strokes);
}

bool QKRadItem::operator >(const QKRadItem &ref) const
{
    if (strokes==ref.strokes)
        return (radical>ref.radical);

    return (strokes>ref.strokes);
}

QKanjiInfo::QKanjiInfo(QChar aKanji, const QStringList &aParts, const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning)
{
    kanji = aKanji;
    parts = aParts;
    onReading = aOnReading;
    kunReading = aKunReading;
    meaning = aMeaning;
}

bool QKanjiInfo::operator ==(const QKanjiInfo &s) const
{
    return (kanji == s.kanji);
}

bool QKanjiInfo::operator !=(const QKanjiInfo &s) const
{
    return (kanji != s.kanji);
}

bool QKanjiInfo::isEmpty() const
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
