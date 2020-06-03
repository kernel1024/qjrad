#include <QDebug>
#include <QCoreApplication>
#include <QString>
#include <QChar>
#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QDomDocument>
#include <QDomElement>
#include "../kdictionary.h"

QHash<QChar,QKanjiInfo> kanjiInfo;
QHash<QChar,QStringList> kanjiParts;
QHash<QChar,int> kanjiStrokes;
QHash<QChar,int> kanjiGrade;

bool parseKanjiDict(const QString& kradfilex, const QString& kanjidict, QTextStream &qrc) {
    // Load radicals list
    QFile fp(kradfilex);
    if (!fp.open(QIODevice::ReadOnly)) {
        qDebug() << "cannot read kanji radicals list";
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
        if (i.nodeName().toLower()!=QSL("character")) continue;
        if (i.firstChildElement(QSL("literal")).isNull() ||
                !i.firstChildElement(QSL("literal")).hasChildNodes() ||
                !i.firstChildElement(QSL("literal")).firstChild().isText() ||
                i.firstChildElement(QSL("literal")).firstChild().toText().data().isEmpty()) {
            qDebug() << "Invalid character entry - no *literal* tag";
            return false;
        }
        QStringList on;
        QStringList kun;
        QStringList mean;

        // literal - kanji character itself
        QChar li = i.firstChildElement(QSL("literal")).firstChild().toText().data().at(0);

        if (li.isHighSurrogate() || li.isLowSurrogate() || !li.isPrint()) continue;

        // stroke count
        if (i.firstChildElement(QSL("misc")).isNull() ||
                !i.firstChildElement(QSL("misc")).hasChildNodes() ||
                i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count")).isNull() ||
                !i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count")).hasChildNodes() ||
                !i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count")).firstChild().isText() ||
                i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count")).firstChild().toText().data().isEmpty()) {
            qDebug() << QSL("Invalid character entry - no *stroke_count* tag, for kanji %1.").arg(li);
            return false;
        }
        bool okconv = false;
        // stroke count
        int lc = i.firstChildElement(QSL("misc")).firstChildElement(QSL("stroke_count"))
                 .firstChild().toText().data().toInt(&okconv);
        if (!okconv) {
            qDebug() << QSL("Invalid character entry - invalid *stroke_count* tag, for kanji %1.").arg(li);
            return false;
        }

        // grade: 1..6 - Kyouiku Kanji, 7..8 - remaining Jouyou Kanji, 9..10 - Jinmeiyou Kanji, 11 - remaining unclassified Kanji
        const int unclassifiedKanjiGrade = 11;
        int lg = unclassifiedKanjiGrade;
        if (!i.firstChildElement(QSL("misc")).isNull() &&
                i.firstChildElement(QSL("misc")).hasChildNodes() &&
                !i.firstChildElement(QSL("misc")).firstChildElement(QSL("grade")).isNull() &&
                i.firstChildElement(QSL("misc")).firstChildElement(QSL("grade")).hasChildNodes() &&
                i.firstChildElement(QSL("misc")).firstChildElement(QSL("grade")).firstChild().isText() &&
                !i.firstChildElement(QSL("misc")).firstChildElement(QSL("grade")).firstChild().toText().data().isEmpty()) {
            lg = i.firstChildElement(QSL("misc")).firstChildElement(QSL("grade")).firstChild().toText().data().toInt(&okconv);
            if (!okconv) lg = unclassifiedKanjiGrade;
        }

        // reading and meaning
        if (!i.firstChildElement(QSL("reading_meaning")).isNull() &&
                i.firstChildElement(QSL("reading_meaning")).hasChildNodes() &&
                !i.firstChildElement(QSL("reading_meaning")).firstChildElement(QSL("rmgroup")).isNull() &&
                i.firstChildElement(QSL("reading_meaning")).firstChildElement(QSL("rmgroup")).hasChildNodes()) {
            for (QDomNode j = i.firstChildElement(QSL("reading_meaning")).firstChildElement(QSL("rmgroup")).firstChild();
                 !j.isNull();j=j.nextSibling()) {
                if (j.nodeName().toLower()==QSL("reading") &&
                        !j.attributes().namedItem(QSL("r_type")).isNull() &&
                        j.attributes().namedItem(QSL("r_type")).nodeValue().toLower()==QSL("ja_on") &&
                        !j.firstChild().isNull() &&
                        j.firstChild().isText() &&
                        !j.firstChild().toText().data().isEmpty()) {
                    on << j.firstChild().toText().data();
                }
                if (j.nodeName().toLower()==QSL("reading") &&
                        !j.attributes().namedItem(QSL("r_type")).isNull() &&
                        j.attributes().namedItem(QSL("r_type")).nodeValue().toLower()==QSL("ja_kun") &&
                        !j.firstChild().isNull() &&
                        j.firstChild().isText() &&
                        !j.firstChild().toText().data().isEmpty()) {
                    kun << j.firstChild().toText().data();
                }
                if (j.nodeName().toLower()==QSL("meaning") &&
                        j.attributes().isEmpty() &&
                        j.firstChild().isText() &&
                        !j.firstChild().toText().data().isEmpty()) {
                    mean << j.firstChild().toText().data();
                }
            }
        }

        QStringList parts;
        if (kanjiParts.contains(li))
            parts = kanjiParts.value(li);

        // add kanji to resources
        QFile res(QSL("k%1.bin").arg(li.unicode()));
        if (res.open(QIODevice::WriteOnly)) {
            QDataStream rout(&res);
            QKanjiInfo ki = QKanjiInfo(li,parts,on,kun,mean);
            rout << ki;
            res.close();
            qrc << QSL("<file alias=\"kanji-%1\">k%1.bin</file>").arg(li.unicode()) << endl;
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

    if (QCoreApplication::arguments().count()!=3) {
        qDebug() << "Kanji dictionary conversion tool";
        qDebug() << "  Usage: kanjicnv <kradfilex> <kanjidict2.xml>";
        qDebug() << " ";
        qDebug() << "This program creates qrc file and over 10000 bin files in current directory";
        qDebug() << "After conversion you can compile qrc file and delete all bin files";
        return 0;
    }

    QFile f(QSL("kanjidic.qrc"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return 1;

    QTextStream fout(&f);
    fout << "<RCC>" << endl << "<qresource prefix=\"/kanjidic/\">" << endl;

    parseKanjiDict(QCoreApplication::arguments().at(1),
                   QCoreApplication::arguments().at(2),fout);

    QFile rstr(QSL("kanjiinfo.bin"));
    if (rstr.open(QIODevice::WriteOnly)) {
        QDataStream sout(&rstr);
        sout << kanjiStrokes << kanjiGrade;
        rstr.close();
        fout << QSL("<file alias=\"summary\">kanjiinfo.bin</file>") << endl;
    }

    fout << "</qresource>" << endl << "</RCC>" << endl;
    fout.flush();
    f.close();
}
