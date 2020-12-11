#include <QProgressDialog>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <algorithm>

#include "kdictionary.h"
#include "global.h"
#include "qsl.h"

const QString kanjiDictFileName     (QSL("dictionary"));
const QString indexFileName         (QSL("index"));
const QString strokesFileName       (QSL("strokes"));
const QString gradeFileName         (QSL("grade"));
const QString radkFileName          (QSL("radkfilex.utf8"));
const QString kradFileName          (QSL("kradfilex.utf8"));
const QString xmlKanjiDictFileName  (QSL("kanjidic2.xml"));
const QString versionFileName       (QSL("version"));

ZKanjiDictionary::ZKanjiDictionary(QObject *parent) :
    QObject(parent)
{
    dataPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dataPath.exists())
        dataPath.mkpath(QSL("."));
}

bool ZKanjiDictionary::loadDictionaries(QWidget *mainWindow)
{
    radicalsLookup.clear();
    kanjiStrokes.clear();
    kanjiGrade.clear();
    errorString.clear();

    if (!isDictionaryDataValid()) {
        if (!setupDictionaryData(mainWindow))
            return false;
    }

    // Load radicals dictionary
    QFile fr(dataPath.filePath(radkFileName));
    if (!fr.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji lookup table");
        return false;
    }
    QTextStream sr(&fr);
    while (!sr.atEnd()) {
        QString s = sr.readLine().trimmed();
        if (s.startsWith('#')) continue; // comment
        if (s.startsWith('$')) { // new radical
            QStringList sl = s.split(' ');
            QChar krad = sl.at(1).at(0);
            bool okconv = false;
            int kst = sl.at(2).toInt(&okconv);
            if (!okconv) kst = 0;
            radicalsLookup << ZKanjiRadicalItem(krad,kst,QString(),QString());
        } else if (!s.isEmpty() && !radicalsLookup.isEmpty()) {
            radicalsLookup.last().kanji += s;
        }
    }
    fr.close();

    // Load radicals list
    QFile fp(dataPath.filePath(kradFileName));
    if (!fp.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanji radicals list");
        return false;
    }
    QTextStream sp(&fp);
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

    QFile fstrokes(dataPath.filePath(strokesFileName));
    if (!fstrokes.open(QIODevice::ReadOnly)) {
        errorString = tr("Unable to load strokes info from dictionary");
        return false;
    }
    kanjiStrokes = ZGlobal::readData(&fstrokes).value<ZKanjiInfoHash>();
    fstrokes.close();

    QFile fgrade(dataPath.filePath(gradeFileName));
    if (!fgrade.open(QIODevice::ReadOnly)) {
        errorString = tr("Unable to load grade info from dictionary");
        return false;
    }
    kanjiGrade = ZGlobal::readData(&fgrade).value<ZKanjiInfoHash>();
    fgrade.close();

    QFile findex(dataPath.filePath(indexFileName));
    if (!findex.open(QIODevice::ReadOnly)) {
        errorString = tr("Unable to load kanji index info from dictionary");
        return false;
    }
    kanjiIndex = ZGlobal::readData(&findex).value<ZKanjiIndex>();
    findex.close();

    return true;
}

bool ZKanjiDictionary::setupDictionaryData(QWidget* mainWindow)
{
    deleteDictionaryData();

    QMessageBox::warning(mainWindow,QGuiApplication::applicationDisplayName(),
                         tr("This is the first start. Please specify directory with Kanji dictionary files:\n"
                            "%1, %2, %3.").arg(xmlKanjiDictFileName,kradFileName,radkFileName));

    const QString fname = QFileDialog::getOpenFileName(mainWindow,tr("KANJIDIC2 XML dictionary"),
                                                       QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                                       tr("KANJIDIC2 file (kanjidic2.xml)"));
    if (fname.isEmpty()) {
        errorString.clear(); // cancel and close application
        return false;
    }

    QFileInfo fiDict(fname);
    QFileInfo fKRad(fiDict.dir().filePath(kradFileName));
    QFileInfo fRadK(fiDict.dir().filePath(radkFileName));

    if (!fiDict.isReadable() || !fKRad.isReadable() || !fRadK.isReadable()) {
        errorString = tr("Unable to open specified dictionary files in %1").arg(fiDict.dir().path());
        return false;
    }

    bool res = QFile::copy(fKRad.filePath(),dataPath.filePath(kradFileName)) &&
               QFile::copy(fRadK.filePath(),dataPath.filePath(radkFileName)) &&
               parseKanjiDict(mainWindow,fname);

    return res;
}

void ZKanjiDictionary::deleteDictionaryData()
{
    QFile f1(dataPath.filePath(kanjiDictFileName));
    QFile f2(dataPath.filePath(indexFileName));
    QFile f3(dataPath.filePath(strokesFileName));
    QFile f4(dataPath.filePath(gradeFileName));
    QFile f5(dataPath.filePath(radkFileName));
    QFile f6(dataPath.filePath(kradFileName));
    QFile f7(dataPath.filePath(versionFileName));

    f1.remove();
    f2.remove();
    f3.remove();
    f4.remove();
    f5.remove();
    f6.remove();
    f7.remove();
}

bool ZKanjiDictionary::isDictionaryDataValid()
{
    const QByteArray qtVersion(QT_VERSION_STR);

    QFile fv(dataPath.filePath(versionFileName));
    if (!fv.open(QIODevice::ReadOnly)) return false;
    if (qtVersion != fv.readAll()) return false;
    fv.close();

    QFileInfo fi1(dataPath.filePath(kanjiDictFileName));
    QFileInfo fi2(dataPath.filePath(indexFileName));
    QFileInfo fi3(dataPath.filePath(strokesFileName));
    QFileInfo fi4(dataPath.filePath(gradeFileName));
    QFileInfo fi5(dataPath.filePath(radkFileName));
    QFileInfo fi6(dataPath.filePath(kradFileName));

    return (fi1.isReadable() &&
            fi2.isReadable() &&
            fi3.isReadable() &&
            fi4.isReadable() &&
            fi5.isReadable() &&
            fi6.isReadable());
}

QString ZKanjiDictionary::sortKanji(const QString &src)
{
    QString s = src;
    std::sort(s.begin(),s.end(),[this](const QChar &c1, const QChar &c2) {
        if (!kanjiStrokes.contains(c1) || !kanjiStrokes.contains(c2)) // also here
            return (c1<c2);

        // in-depth compare with kdict info
        if (kanjiStrokes.value(c1) != kanjiStrokes.value(c2)) // if strokes count differs...
            return (kanjiStrokes.value(c1) < kanjiStrokes.value(c2)); // compare by strokes count

        if (kanjiGrade.value(c1) != kanjiGrade.value(c2)) // if grade level differs...
            return (kanjiGrade.value(c1) != kanjiGrade.value(c2)); // compare by grade

        return (c1<c2); // compare by unicode code inside same-grade/same-strokes groups
    });
    return s;
}

ZKanjiInfo ZKanjiDictionary::getKanjiInfo(QChar kanji)
{
    auto idx = kanjiIndex.value(kanji.unicode(),-1L);

    if (idx < 0L)
        return ZKanjiInfo();

    QFile fdict(dataPath.filePath(kanjiDictFileName));
    if (!fdict.open(QIODevice::ReadOnly))
        return ZKanjiInfo();

    fdict.seek(idx);
    auto ki = ZGlobal::readData(&fdict,QVariant::fromValue(ZKanjiInfo())).value<ZKanjiInfo>();
    fdict.close();
    return ki;
}

QString ZKanjiDictionary::getErrorString() const
{
    return errorString;
}

void ZKanjiDictionary::cleanupDictionaries()
{
    deleteDictionaryData();
    zF->deferredQuit();
}

bool ZKanjiDictionary::parseKanjiDict(QWidget* mainWindow, const QString &xmlDictFileName)
{
    QDomDocument kanjiDict;
    QFile fk(xmlDictFileName);
    if (!fk.open(QIODevice::ReadOnly)) {
        errorString = tr("cannot read kanjidict");
        return false;
    }
    if (!kanjiDict.setContent(&fk)) {
        fk.close();
        errorString = tr("cannot parse kanjidict xml");
        return false;
    }

    ZKanjiIndex index;
    ZKanjiInfoHash strokes;
    ZKanjiInfoHash grade;

    QFile fkdict(dataPath.filePath(kanjiDictFileName));
    if (!fkdict.open(QIODevice::WriteOnly)) {
        errorString = tr("Unable to create kanji dictionary file");
        return false;
    }

    const QDomElement root = kanjiDict.documentElement();

    const int progressDiv = 5;
    int childIdx = 0;
    QProgressDialog dlg(tr("Parsing %1").arg(kanjiDictFileName),tr("Cancel"),
                        0,root.childNodes().count()/progressDiv,mainWindow);
    dlg.setMinimumDuration(0);
    dlg.setWindowModality(Qt::WindowModal);
    QApplication::processEvents();

    for (QDomNode i = root.firstChild();!i.isNull();i=i.nextSibling()) {
        childIdx++;
        if (dlg.value() != (childIdx/progressDiv))
            dlg.setValue(childIdx/progressDiv);

        if (dlg.wasCanceled()) {
            errorString = tr("Kanji dictionary parsing was cancelled by user.");
            fkdict.close();
            fkdict.remove();
            return false;
        }

        if (i.nodeName().toLower()!=QSL("character")) continue;

        // literal - kanji character itself
        const QString literal = i.firstChildElement(QSL("literal")).firstChild().toText().data();
        if (literal.isEmpty()) {
            errorString = tr("Invalid character entry - no *literal* tag");
            fkdict.close();
            fkdict.remove();
            return false;
        }
        const QChar li = literal.at(0);

        if (li.isHighSurrogate() || li.isLowSurrogate() || !li.isPrint()) continue;

        // stroke count
        bool okconv = false;
        const QString strokeCnt = i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count"))
                                  .firstChild().toText().data();
        const int lc = strokeCnt.toInt(&okconv);
        if (strokeCnt.isEmpty() || !okconv) {
            errorString = QSL("Invalid character entry - no *stroke_count* tag, for kanji %1.").arg(li);
            fkdict.close();
            fkdict.remove();
            return false;
        }

        // grade: 1..6 - Kyouiku Kanji, 7..8 - remaining Jouyou Kanji, 9..10 - Jinmeiyou Kanji, 11 - remaining unclassified Kanji
        const int unclassifiedKanjiGrade = 11;
        int lg = i.firstChildElement(QSL("misc")).firstChildElement(QSL("grade")).firstChild().toText().data().toInt(&okconv);
        if (!okconv)
            lg = unclassifiedKanjiGrade;

        // reading and meaning
        const QDomNode rmgroup = i.firstChildElement(QSL("reading_meaning")).firstChildElement(QSL("rmgroup"));
        QStringList on;
        on.reserve(rmgroup.childNodes().count());
        QStringList kun;
        kun.reserve(rmgroup.childNodes().count());
        QStringList mean;
        mean.reserve(rmgroup.childNodes().count());

        for (QDomNode j = rmgroup.firstChild(); !j.isNull(); j=j.nextSibling()) {
            if (j.nodeName().toLower()==QSL("reading") &&
                    j.attributes().namedItem(QSL("r_type")).nodeValue().toLower()==QSL("ja_on") &&
                    !j.firstChild().toText().data().isEmpty()) {
                on.append(j.firstChild().toText().data());
            }
            if (j.nodeName().toLower()==QSL("reading") &&
                    j.attributes().namedItem(QSL("r_type")).nodeValue().toLower()==QSL("ja_kun") &&
                    !j.firstChild().toText().data().isEmpty()) {
                kun.append(j.firstChild().toText().data());
            }
            if (j.nodeName().toLower()==QSL("meaning") &&
//                    j.attributes().isEmpty() &&
                    !j.firstChild().toText().data().isEmpty()) {
                mean.append(j.firstChild().toText().data());
            }
        }

        QStringList parts;
        if (kanjiParts.contains(li))
            parts = kanjiParts.value(li);

        qint64 offset = fkdict.pos();
        qint64 size = ZGlobal::writeData(&fkdict,QVariant::fromValue(ZKanjiInfo(li,parts,on,kun,mean)));
        if (size <= 0) {
            errorString = QSL("Unable to write kanji to dictionary.");
            fkdict.close();
            fkdict.remove();
            return false;
        }

        index[li.unicode()] = offset;
        strokes[li] = lc;
        grade[li] = lg;
    }
    fkdict.close();

    QFile fidx(dataPath.filePath(indexFileName));
    if (!fidx.open(QIODevice::WriteOnly)) {
        errorString = QSL("Unable to create Kanji index file.");
        fkdict.remove();
        return false;
    }
    ZGlobal::writeData(&fidx,QVariant::fromValue(index));
    fidx.close();

    QFile fstrokes(dataPath.filePath(strokesFileName));
    if (!fstrokes.open(QIODevice::WriteOnly)) {
        errorString = QSL("Unable to create Kanji strokes file.");
        fkdict.remove();
        fidx.remove();
        return false;
    }
    ZGlobal::writeData(&fstrokes,QVariant::fromValue(strokes));
    fstrokes.close();

    QFile fgrade(dataPath.filePath(gradeFileName));
    if (!fgrade.open(QIODevice::WriteOnly)) {
        errorString = QSL("Unable to create Kanji grade file.");
        fkdict.remove();
        fidx.remove();
        fstrokes.remove();
        return false;
    }
    ZGlobal::writeData(&fgrade,QVariant::fromValue(grade));
    fgrade.close();

    const QByteArray qtVersion(QT_VERSION_STR);

    QFile fversion(dataPath.filePath(versionFileName));
    if (!fversion.open(QIODevice::WriteOnly)) {
        errorString = QSL("Unable to create Kanji dictionary version file.");
        fgrade.remove();
        fkdict.remove();
        fidx.remove();
        fstrokes.remove();
        return false;
    }
    fversion.write(qtVersion);
    fversion.close();

    return true;
}

ZKanjiRadicalItem::ZKanjiRadicalItem(QChar aRadical)
{
    radical = aRadical;
    strokes = 0;
    jisCode.clear();
    kanji.clear();
}

ZKanjiRadicalItem::ZKanjiRadicalItem(QChar aRadical, int aStrokes)
{
    radical = aRadical;
    strokes = aStrokes;
    jisCode.clear();
    kanji.clear();
}

ZKanjiRadicalItem::ZKanjiRadicalItem(QChar aRadical, int aStrokes, const QString &aJisCode, const QString &aKanji)
{
    radical = aRadical;
    strokes = aStrokes;
    jisCode = aJisCode;
    kanji = aKanji;
}

bool ZKanjiRadicalItem::operator ==(const ZKanjiRadicalItem &s) const
{
    return (radical==s.radical);
}

bool ZKanjiRadicalItem::operator !=(const ZKanjiRadicalItem &s) const
{
    return (radical!=s.radical);
}

bool ZKanjiRadicalItem::operator <(const ZKanjiRadicalItem &ref) const
{
    if (strokes==ref.strokes)
        return (radical<ref.radical);

    return (strokes<ref.strokes);
}

bool ZKanjiRadicalItem::operator >(const ZKanjiRadicalItem &ref) const
{
    if (strokes==ref.strokes)
        return (radical>ref.radical);

    return (strokes>ref.strokes);
}

ZKanjiInfo::ZKanjiInfo(QChar aKanji, const QStringList &aParts, const QStringList &aOnReading, const QStringList &aKunReading, const QStringList &aMeaning)
{
    kanji = aKanji;
    parts = aParts;
    onReading = aOnReading;
    kunReading = aKunReading;
    meaning = aMeaning;
}

bool ZKanjiInfo::operator ==(const ZKanjiInfo &s) const
{
    return (kanji == s.kanji);
}

bool ZKanjiInfo::operator !=(const ZKanjiInfo &s) const
{
    return (kanji != s.kanji);
}

bool ZKanjiInfo::isEmpty() const
{
    return (kanji.isNull());
}

QDataStream &operator <<(QDataStream &out, const ZKanjiInfo &obj)
{
    out << obj.kanji << obj.parts << obj.onReading << obj.kunReading << obj.meaning;
    return out;
}


QDataStream &operator >>(QDataStream &in, ZKanjiInfo &obj)
{
    in >> obj.kanji >> obj.parts >> obj.onReading >> obj.kunReading >> obj.meaning;
    return in;
}
