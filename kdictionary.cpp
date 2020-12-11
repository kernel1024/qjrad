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
    m_dataPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!m_dataPath.exists())
        m_dataPath.mkpath(QSL("."));
}

bool ZKanjiDictionary::loadDictionaries(QWidget *mainWindow)
{
    m_radicalsList.clear();
    m_radicalsLookup.clear();
    m_kanjiStrokes.clear();
    m_kanjiGrade.clear();
    m_errorString.clear();

    if (!isDictionaryDataValid()) {
        if (!setupDictionaryData(mainWindow))
            return false;
    }

    // Load radicals dictionary
    QFile fr(m_dataPath.filePath(radkFileName));
    if (!fr.open(QIODevice::ReadOnly)) {
        m_errorString = tr("cannot read kanji lookup table");
        return false;
    }
    QTextStream sr(&fr);
    QChar krad;
    int kst = 0;
    while (!sr.atEnd()) {
        QString s = sr.readLine().trimmed();
        if (s.startsWith('#')) continue; // comment
        if (s.startsWith('$')) { // new radical
            QStringList sl = s.split(' ');
            krad = sl.at(1).at(0);
            bool okconv = false;
            kst = sl.at(2).toInt(&okconv);
            if (!okconv) kst = 0;
        } else if (!s.isEmpty() && !krad.isNull()) {
            if (m_radicalsLookup.contains(krad)) {
                m_radicalsLookup[krad].kanji += s;
            } else {
                m_radicalsLookup[krad] = ZKanjiRadicalItem(kst,s);
                m_radicalsList.append(qMakePair(krad,kst));
            }
        }
    }
    fr.close();

    // Load radicals list
    QFile fp(m_dataPath.filePath(kradFileName));
    if (!fp.open(QIODevice::ReadOnly)) {
        m_errorString = tr("cannot read kanji radicals list");
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
            m_kanjiParts[k] = sl.join(QString());
        }
    }
    fp.close();

    QFile fstrokes(m_dataPath.filePath(strokesFileName));
    if (!fstrokes.open(QIODevice::ReadOnly)) {
        m_errorString = tr("Unable to load strokes info from dictionary");
        return false;
    }
    m_kanjiStrokes = ZGlobal::readData(&fstrokes).value<ZKanjiInfoHash>();
    fstrokes.close();

    QFile fgrade(m_dataPath.filePath(gradeFileName));
    if (!fgrade.open(QIODevice::ReadOnly)) {
        m_errorString = tr("Unable to load grade info from dictionary");
        return false;
    }
    m_kanjiGrade = ZGlobal::readData(&fgrade).value<ZKanjiInfoHash>();
    fgrade.close();

    QFile findex(m_dataPath.filePath(indexFileName));
    if (!findex.open(QIODevice::ReadOnly)) {
        m_errorString = tr("Unable to load kanji index info from dictionary");
        return false;
    }
    m_kanjiIndex = ZGlobal::readData(&findex).value<ZKanjiIndex>();
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
        m_errorString.clear(); // cancel and close application
        return false;
    }

    QFileInfo fiDict(fname);
    QFileInfo fKRad(fiDict.dir().filePath(kradFileName));
    QFileInfo fRadK(fiDict.dir().filePath(radkFileName));

    if (!fiDict.isReadable() || !fKRad.isReadable() || !fRadK.isReadable()) {
        m_errorString = tr("Unable to open specified dictionary files in %1").arg(fiDict.dir().path());
        return false;
    }

    bool res = QFile::copy(fKRad.filePath(),m_dataPath.filePath(kradFileName)) &&
               QFile::copy(fRadK.filePath(),m_dataPath.filePath(radkFileName)) &&
               parseKanjiDict(mainWindow,fname);

    return res;
}

void ZKanjiDictionary::deleteDictionaryData()
{
    QFile f1(m_dataPath.filePath(kanjiDictFileName));
    QFile f2(m_dataPath.filePath(indexFileName));
    QFile f3(m_dataPath.filePath(strokesFileName));
    QFile f4(m_dataPath.filePath(gradeFileName));
    QFile f5(m_dataPath.filePath(radkFileName));
    QFile f6(m_dataPath.filePath(kradFileName));
    QFile f7(m_dataPath.filePath(versionFileName));

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

    QFile fv(m_dataPath.filePath(versionFileName));
    if (!fv.open(QIODevice::ReadOnly)) return false;
    if (qtVersion != fv.readAll()) return false;
    fv.close();

    QFileInfo fi1(m_dataPath.filePath(kanjiDictFileName));
    QFileInfo fi2(m_dataPath.filePath(indexFileName));
    QFileInfo fi3(m_dataPath.filePath(strokesFileName));
    QFileInfo fi4(m_dataPath.filePath(gradeFileName));
    QFileInfo fi5(m_dataPath.filePath(radkFileName));
    QFileInfo fi6(m_dataPath.filePath(kradFileName));

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
        if (!m_kanjiStrokes.contains(c1) || !m_kanjiStrokes.contains(c2)) // also here
            return (c1<c2);

        // in-depth compare with kdict info
        if (m_kanjiStrokes.value(c1) != m_kanjiStrokes.value(c2)) // if strokes count differs...
            return (m_kanjiStrokes.value(c1) < m_kanjiStrokes.value(c2)); // compare by strokes count

        if (m_kanjiGrade.value(c1) != m_kanjiGrade.value(c2)) // if grade level differs...
            return (m_kanjiGrade.value(c1) != m_kanjiGrade.value(c2)); // compare by grade

        return (c1<c2); // compare by unicode code inside same-grade/same-strokes groups
    });
    return s;
}

ZKanjiInfo ZKanjiDictionary::getKanjiInfo(QChar kanji)
{
    auto idx = m_kanjiIndex.value(kanji.unicode(),-1L);

    if (idx < 0L)
        return ZKanjiInfo();

    QFile fdict(m_dataPath.filePath(kanjiDictFileName));
    if (!fdict.open(QIODevice::ReadOnly))
        return ZKanjiInfo();

    fdict.seek(idx);
    auto ki = ZGlobal::readData(&fdict,QVariant::fromValue(ZKanjiInfo())).value<ZKanjiInfo>();
    fdict.close();
    return ki;
}

QString ZKanjiDictionary::getErrorString() const
{
    return m_errorString;
}

void ZKanjiDictionary::cleanupDictionaries()
{
    deleteDictionaryData();
    zF->deferredQuit();
}

const QList<QPair<QChar, int> > &ZKanjiDictionary::getAllRadicals() const
{
    return m_radicalsList;
}

ZKanjiRadicalItem ZKanjiDictionary::getRadicalInfo(const QChar &radical) const
{
    return m_radicalsLookup.value(radical);
}

QString ZKanjiDictionary::lookupRadicals(const QString &radicals) const
{
    QStringList res;
    res.reserve(radicals.length());
    for (const auto &rad : radicals)
        res.append(m_radicalsLookup.value(rad).kanji);

    // leave only kanji present for all selected radicals
    // TODO: optimize this!
    while (res.count()>1) {
        QString fs = res.at(0);
        QString ms = res.takeLast();
        int i=0;
        while (i<fs.length()) {
            if (!ms.contains(fs.at(i))) {
                fs = fs.remove(i,1);
            } else {
                i++;
            }
        }
        if (fs.isEmpty()) {
            res.clear();
            res << QString();
            break;
        }
        res.replace(0,fs);
    }

    if (res.isEmpty())
        return QString();

    return res.first();
}

QString ZKanjiDictionary::getKanjiParts(const QChar &kanji) const
{
    return m_kanjiParts.value(kanji);
}

bool ZKanjiDictionary::parseKanjiDict(QWidget* mainWindow, const QString &xmlDictFileName)
{
    QDomDocument kanjiDict;
    QFile fk(xmlDictFileName);
    if (!fk.open(QIODevice::ReadOnly)) {
        m_errorString = tr("cannot read kanjidict");
        return false;
    }
    if (!kanjiDict.setContent(&fk)) {
        fk.close();
        m_errorString = tr("cannot parse kanjidict xml");
        return false;
    }

    ZKanjiIndex index;
    ZKanjiInfoHash strokes;
    ZKanjiInfoHash grade;

    QFile fkdict(m_dataPath.filePath(kanjiDictFileName));
    if (!fkdict.open(QIODevice::WriteOnly)) {
        m_errorString = tr("Unable to create kanji dictionary file");
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
            m_errorString = tr("Kanji dictionary parsing was cancelled by user.");
            fkdict.close();
            fkdict.remove();
            return false;
        }

        if (i.nodeName().toLower()!=QSL("character")) continue;

        // literal - kanji character itself
        const QString literal_s = i.firstChildElement(QSL("literal")).firstChild().toText().data();
        if (literal_s.isEmpty()) {
            m_errorString = tr("Invalid character entry - no *literal* tag");
            fkdict.close();
            fkdict.remove();
            return false;
        }
        const QChar literal = literal_s.at(0);

        if (literal.isHighSurrogate() || literal.isLowSurrogate() || !literal.isPrint()) continue;

        // stroke count
        bool okconv = false;
        const QString strokeCnt = i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count"))
                                  .firstChild().toText().data();
        const int lc = strokeCnt.toInt(&okconv);
        if (strokeCnt.isEmpty() || !okconv) {
            m_errorString = QSL("Invalid character entry - no *stroke_count* tag, for kanji %1.").arg(literal);
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
                    j.attributes().isEmpty() &&
                    !j.firstChild().toText().data().isEmpty()) {
                mean.append(j.firstChild().toText().data());
            }
        }

        qint64 offset = fkdict.pos();
        qint64 size = ZGlobal::writeData(&fkdict,QVariant::fromValue(ZKanjiInfo(literal,on,kun,mean)));
        if (size <= 0) {
            m_errorString = QSL("Unable to write kanji to dictionary.");
            fkdict.close();
            fkdict.remove();
            return false;
        }

        index[literal.unicode()] = offset;
        strokes[literal] = lc;
        grade[literal] = lg;
    }
    fkdict.close();

    QFile fidx(m_dataPath.filePath(indexFileName));
    if (!fidx.open(QIODevice::WriteOnly)) {
        m_errorString = QSL("Unable to create Kanji index file.");
        fkdict.remove();
        return false;
    }
    ZGlobal::writeData(&fidx,QVariant::fromValue(index));
    fidx.close();

    QFile fstrokes(m_dataPath.filePath(strokesFileName));
    if (!fstrokes.open(QIODevice::WriteOnly)) {
        m_errorString = QSL("Unable to create Kanji strokes file.");
        fkdict.remove();
        fidx.remove();
        return false;
    }
    ZGlobal::writeData(&fstrokes,QVariant::fromValue(strokes));
    fstrokes.close();

    QFile fgrade(m_dataPath.filePath(gradeFileName));
    if (!fgrade.open(QIODevice::WriteOnly)) {
        m_errorString = QSL("Unable to create Kanji grade file.");
        fkdict.remove();
        fidx.remove();
        fstrokes.remove();
        return false;
    }
    ZGlobal::writeData(&fgrade,QVariant::fromValue(grade));
    fgrade.close();

    const QByteArray qtVersion(QT_VERSION_STR);

    QFile fversion(m_dataPath.filePath(versionFileName));
    if (!fversion.open(QIODevice::WriteOnly)) {
        m_errorString = QSL("Unable to create Kanji dictionary version file.");
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

int ZKanjiDictionary::getKanjiGrade(const QChar &kanji) const
{
    if (m_kanjiGrade.contains(kanji))
        return m_kanjiGrade.value(kanji);

    return 0;
}

int ZKanjiDictionary::getKanjiStrokes(const QChar &kanji) const
{
    if (m_kanjiStrokes.contains(kanji))
        return m_kanjiStrokes.value(kanji);

    return 0;
}

ZKanjiInfo::ZKanjiInfo(QChar aKanji, const QStringList &aOnReading, const QStringList &aKunReading,
                       const QStringList &aMeaning)
{
    kanji = aKanji;
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
    out << obj.kanji << obj.onReading << obj.kunReading << obj.meaning;
    return out;
}


QDataStream &operator >>(QDataStream &in, ZKanjiInfo &obj)
{
    in >> obj.kanji >> obj.onReading >> obj.kunReading >> obj.meaning;
    return in;
}

ZKanjiRadicalItem::ZKanjiRadicalItem(int aStrokes, const QString &aKanji)
{
    strokes = aStrokes;
    kanji = aKanji;
}
