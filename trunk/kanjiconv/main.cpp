#include <QtCore>
#include "../kdictionary.h"

QHash<QChar,QKanjiInfo> kanjiInfo;
QHash<QChar,QStringList> kanjiParts;
QHash<QChar,int> kanjiStrokes;
QHash<QChar,int> kanjiGrade;

bool parseKanjiDict(QString kradfilex, QString kanjidict, QTextStream &qrc) {
    // Load radicals list
    QFile fp(kradfilex);
    if (!fp.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot read kanji radicals list";
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
    QFile fk(kanjidict);
    if (!fk.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot read kanjidict";
        return false;
    }
    if (!kanjiDict.setContent(&fk)) {
        fk.close();
        qDebug() << "cannot parse kanjidict xml";
        return false;
    }

    QDomElement root = kanjiDict.documentElement();

    for (QDomNode i = root.firstChild();!i.isNull();i=i.nextSibling()) {
        if (i.nodeName().toLower()!="character") continue;
        if (i.firstChildElement("literal").isNull() ||
                !i.firstChildElement("literal").hasChildNodes() ||
                !i.firstChildElement("literal").firstChild().isText() ||
                i.firstChildElement("literal").firstChild().toText().data().isEmpty()) {
            qDebug() << "Invalid character entry - no *literal* tag";
            return false;
        }
        QStringList on, kun, mean;
        on.clear();
        kun.clear();
        mean.clear();

        // literal - kanji character itself
        QChar li = i.firstChildElement("literal").firstChild().toText().data().at(0);

        if (li.isHighSurrogate() || li.isLowSurrogate() || !li.isPrint()) continue;

        // stroke count
        if (i.firstChildElement("misc").isNull() ||
                !i.firstChildElement("misc").hasChildNodes() ||
                i.firstChildElement("misc").firstChildElement("stroke_count").isNull() ||
                !i.firstChildElement("misc").firstChildElement("stroke_count").hasChildNodes() ||
                !i.firstChildElement("misc").firstChildElement("stroke_count").firstChild().isText() ||
                i.firstChildElement("misc").firstChildElement("stroke_count").firstChild().toText().data().isEmpty()) {
            qDebug() << QString("Invalid character entry - no *stroke_count* tag, for kanji %1.").arg(li);
            return false;
        }
        bool okconv;
        // stroke count
        int lc = i.firstChildElement("misc").firstChildElement("stroke_count").firstChild().toText().data().toInt(&okconv);
        if (!okconv) {
            qDebug() << QString("Invalid character entry - invalid *stroke_count* tag, for kanji %1.").arg(li);
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

        // add kanji to resources
        QFile res(QString("k%1.bin").arg(li.unicode()));
        if (res.open(QIODevice::WriteOnly)) {
            QDataStream rout(&res);
            QKanjiInfo ki = QKanjiInfo(li,parts,on,kun,mean);
            rout << ki;
            res.close();
            qrc << QString("<file alias=\"kanji-%1\">k%1.bin</file>").arg(li.unicode()) << endl;
            kanjiStrokes[li] = lc;
            kanjiGrade[li] = lg;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<QKRadItem>("QKRadItem");
    qRegisterMetaType<QKanjiInfo>("QKanjiInfo");
    qRegisterMetaTypeStreamOperators<QKanjiInfo>("QKanjiInfo");
    QCoreApplication a(argc, argv);

    if (a.arguments().count()!=3) {
        qDebug() << "Kanji dictionary conversion tool";
        qDebug() << "  Usage: kanjicnv <kradfilex> <kanjidict2.xml>";
        qDebug() << " ";
        qDebug() << "This program creates qrc file and over 10000 bin files in current directory";
        qDebug() << "After conversion you can compile qrc file and delete all bin files";
        return 0;
    }

    QFile f("kanjidic.qrc");
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return 1;

    QTextStream fout(&f);
    fout << "<RCC>" << endl << "<qresource prefix=\"/kanjidic/\">" << endl;

    parseKanjiDict(a.arguments().at(1),a.arguments().at(2),fout);

    QFile rstr(QString("kanjiinfo.bin"));
    if (rstr.open(QIODevice::WriteOnly)) {
        QDataStream sout(&rstr);
        sout << kanjiStrokes << kanjiGrade;
        rstr.close();
        fout << QString("<file alias=\"summary\">kanjiinfo.bin</file>") << endl;
    }

    fout << "</qresource>" << endl << "</RCC>" << endl;
    fout.flush();
    f.close();
}
