#include "kdictionary.h"

QKDictionary::QKDictionary(QObject *parent) :
    QObject(parent)
{
    radicalsLookup.clear();
    kanjiDict.clear();
}

bool QKDictionary::loadDictionaries()
{
    radicalsLookup.clear();
    kanjiDict.clear();

    // Load radicals dictionary
    QFile fr(":/data/radkfilex");
    if (!fr.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot read kanji lookup table";
        return false;
    }
    QTextStream sr(&fr);
    sr.setCodec("EUC-JP");
    while (!sr.atEnd()) {
        QString s = sr.readLine().trimmed();
        if (s.startsWith('#')) continue; // comment
        else if (s.startsWith('$')) { // new radical
            QStringList sp = s.split(' ');
            QChar krad = sp.at(1).at(0);
            bool okconv;
            int kst = sp.at(2).toInt(&okconv);
            if (!okconv) kst = 0;
            radicalsLookup << QKRadItem(krad,kst,"","");
        } else if (!s.isEmpty() && !radicalsLookup.isEmpty()) {
            radicalsLookup.last().kanji += s;
        }
    }
    fr.close();

    QFile fk(":/data/kanjidic2.xml");
    if (!fk.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot read kanjidict";
        return false;
    }
    if (!kanjiDict.setContent(&fk)) {
        fk.close();
        qDebug() << "cannot parse kanjidict xml";
        return false;
    }

    // create kanjiStrokes hash

    return true;
}

QKRadItem::QKRadItem()
{
    radical = '\0';
    strokes = 0;
    jisCode.clear();
    kanji.clear();
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
