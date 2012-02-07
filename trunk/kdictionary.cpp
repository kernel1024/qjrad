#include "kdictionary.h"

QKDictionary::QKDictionary(QObject *parent) :
    QObject(parent)
{
    radicalsLookup.clear();
    kanjiInfo.clear();
    kanjiParts.clear();
    errorString.clear();
}

bool QKDictionary::loadDictionaries()
{
    radicalsLookup.clear();
    errorString.clear();

    // Load radicals dictionary
    QFile fr(":/data/radkfilex");
    if (!fr.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji lookup table");
        return false;
    }
    QTextStream sr(&fr);
    sr.setCodec("EUC-JP");
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
    QFile fp(":/data/kradfilex");
    if (!fp.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji radicals list");
        return false;
    }
    QTextStream sp(&fp);
    sp.setCodec("EUC-JP");
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

    QDomDocument kanjiDict;
    QFile fk(":/data/kanjidic2.xml");
    if (!fk.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanjidict");
        return false;
    }
    if (!kanjiDict.setContent(&fk)) {
        fk.close();
        errorString = tr("cannot parse kanjidict xml");
        return false;
    }

    QDomElement root = kanjiDict.documentElement();

    for (QDomNode i = root.firstChild();!i.isNull();i=i.nextSibling()) {
        if (i.nodeName().toLower()!="character") continue;
        if (i.firstChildElement("literal").isNull() ||
                !i.firstChildElement("literal").hasChildNodes() ||
                !i.firstChildElement("literal").firstChild().isText() ||
                i.firstChildElement("literal").firstChild().toText().data().isEmpty()) {
            errorString = tr("Invalid character entry - no *literal* tag");
            return false;
        }
        QStringList on, kun, mean;
        on.clear();
        kun.clear();
        mean.clear();

        // literal - kanji character itself
        QChar li = i.firstChildElement("literal").firstChild().toText().data().at(0);

        // stroke count
        if (i.firstChildElement("misc").isNull() ||
                !i.firstChildElement("misc").hasChildNodes() ||
                i.firstChildElement("misc").firstChildElement("stroke_count").isNull() ||
                !i.firstChildElement("misc").firstChildElement("stroke_count").hasChildNodes() ||
                !i.firstChildElement("misc").firstChildElement("stroke_count").firstChild().isText() ||
                i.firstChildElement("misc").firstChildElement("stroke_count").firstChild().toText().data().isEmpty()) {
            errorString = tr("Invalid character entry - no *stroke_count* tag, for kanji %1.").arg(li);
            return false;
        }
        bool okconv;
        // stroke count
        int lc = i.firstChildElement("misc").firstChildElement("stroke_count").firstChild().toText().data().toInt(&okconv);
        if (!okconv) {
            errorString = tr("Invalid character entry - invalid *stroke_count* tag, for kanji %1.").arg(li);
            return false;
        }

        // grade: 1..6 - Kyouiku Kanji, 7..8 - remaining Jouyou Kanji, 9..10 - Jinmeiyou Kanji, 11 - remaining unclassified Kanji
        int lg=11;
        if (!i.firstChildElement("misc").isNull() &&
                i.firstChildElement("misc").hasChildNodes() &&
                !i.firstChildElement("misc").firstChildElement("grade").isNull() &&
                i.firstChildElement("misc").firstChildElement("grade").hasChildNodes() &&
                i.firstChildElement("misc").firstChildElement("grade").firstChild().isText() &&
                !i.firstChildElement("misc").firstChildElement("grade").firstChild().toText().data().isEmpty()) {
            lg = i.firstChildElement("misc").firstChildElement("grade").firstChild().toText().data().toInt(&okconv);
            if (!okconv) lg=11;
        }

        // reading and meaning
        if (!i.firstChildElement("reading_meaning").isNull() &&
                i.firstChildElement("reading_meaning").hasChildNodes() &&
                !i.firstChildElement("reading_meaning").firstChildElement("rmgroup").isNull() &&
                i.firstChildElement("reading_meaning").firstChildElement("rmgroup").hasChildNodes()) {
            for (QDomNode j = i.firstChildElement("reading_meaning").firstChildElement("rmgroup").firstChild();!j.isNull();j=j.nextSibling()) {
                if (j.nodeName().toLower()=="reading" &&
                        !j.attributes().namedItem("r_type").isNull() &&
                        j.attributes().namedItem("r_type").nodeValue().toLower()=="ja_on" &&
                        !j.firstChild().isNull() &&
                        j.firstChild().isText() &&
                        !j.firstChild().toText().data().isEmpty()) {
                    on << j.firstChild().toText().data();
                }
                if (j.nodeName().toLower()=="reading" &&
                        !j.attributes().namedItem("r_type").isNull() &&
                        j.attributes().namedItem("r_type").nodeValue().toLower()=="ja_kun" &&
                        !j.firstChild().isNull() &&
                        j.firstChild().isText() &&
                        !j.firstChild().toText().data().isEmpty()) {
                    kun << j.firstChild().toText().data();
                }
                if (j.nodeName().toLower()=="meaning" &&
                        j.attributes().isEmpty() &&
                        j.firstChild().isText() &&
                        !j.firstChild().toText().data().isEmpty()) {
                    mean << j.firstChild().toText().data();
                }
            }
        }

        QStringList parts;
        parts.clear();
        if (kanjiParts.contains(li))
            parts = kanjiParts[li];

        kanjiInfo[li] = QKanjiInfo(li,lc,parts,on,kun,mean,lg);
    }

    kanjiDict.clear();

    return true;
}

QKDictionary *kdict = NULL;
QMutex kdictmutex;

// Kanji sorting magic
bool kanjiLessThan(const QChar &c1, const QChar &c2)
{
    if (kdict==NULL) return (c1<c2); // in case if kdict is not present - simply compare unicode characters
    if (!kdict->kanjiInfo.contains(c1) || !kdict->kanjiInfo.contains(c2)) return (c1<c2); // also here
    // in-depth compare with kdict info
    if (kdict->kanjiInfo[c1].strokes!=kdict->kanjiInfo[c2].strokes) // if strokes count differs...
        return (kdict->kanjiInfo[c1].strokes<kdict->kanjiInfo[c2].strokes); // compare by strokes count
    else {
        if (kdict->kanjiInfo[c1].grade!=kdict->kanjiInfo[c2].grade) // if grade level differs...
            return (kdict->kanjiInfo[c1].grade!=kdict->kanjiInfo[c2].grade); // compare by grade
        else
            return (c1<c2); // compare by unicode code inside same-grade/same-strokes groups
    }

}

QString QKDictionary::sortKanji(const QString src)
{
    kdictmutex.lock();
    kdict = this;
    QString s = src;
    qSort(s.begin(),s.end(),kanjiLessThan);
    kdictmutex.unlock();
    return s;
}

QKRadItem::QKRadItem()
{
    radical = QChar();
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

QKanjiInfo::QKanjiInfo()
{
    kanji = QChar();
    strokes = 0;
    parts.clear();
    onReading.clear();
    kunReading.clear();
    meaning.clear();
}

QKanjiInfo::QKanjiInfo(const QChar &aKanji, int aStrokes, const QStringList &aParts, const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning, int aGrade)
{
    kanji = aKanji;
    strokes = aStrokes;
    parts = aParts;
    onReading = aOnReading;
    kunReading = aKunReading;
    meaning = aMeaning;
    grade = aGrade;
}

QKanjiInfo &QKanjiInfo::operator =(const QKanjiInfo &other)
{
    kanji = other.kanji;
    strokes = other.strokes;
    parts = other.parts;
    onReading = other.onReading;
    kunReading = other.kunReading;
    meaning = other.meaning;
    grade = other.grade;
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


