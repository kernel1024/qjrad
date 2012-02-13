#include "kanjimodel.h"
#include "miscutils.h"

QKanjiModel::QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont,
                         const QFont &MarkFont, QHash<QChar,QKanjiInfo> *KanjiInfo) :
    QAbstractListModel(parent)
{
    kanjiList = KanjiList;
    kanjiFont = KanjiFont;
    kanjiInfo = KanjiInfo;
    markFont = MarkFont;
}

Qt::ItemFlags QKanjiModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.row()<kanjiList.length()) {
            QChar k = kanjiList.at(index.row());
            if (!isRegularKanji(k))
                return Qt::ItemIsEnabled;
        }
    }

    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QVariant QKanjiModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (index.row()>=kanjiList.length()) return QVariant();
    QChar k = kanjiList.at(index.row());

    if (role == Qt::DecorationRole) {
        QColor bkgd = QApplication::palette("QListView").color(QPalette::Base);
        QColor fgd = QApplication::palette("QListView").color(QPalette::Text);
        QColor njfgd = middleColor(bkgd,fgd,60);

        QFontMetrics fm(kanjiFont);
        int sz = 12*fm.width(QChar(0x9fa0))/10; // yaku/fue, biggest radical
        QPixmap px(sz,sz);
        QPainter pn(&px);
        pn.setFont(kanjiFont);
        pn.fillRect(0,0,sz,sz,bkgd);

        if (!isRegularKanji(k)) { // this is a unselectable mark, not kanji
            int v = k.unicode()-0x2460;
            pn.setFont(markFont);
            pn.setPen(QPen(njfgd));
            QVector<QLine> rrct;
            createHxBox(rrct,sz,3);
            pn.drawLines(rrct);
            pn.setPen(QPen(fgd));
            pn.drawText(0,0,sz-1,sz-1,Qt::AlignCenter,tr("%1").arg(v));
        } else { // this is regular kanji
            if ((*kanjiInfo)[k].grade<=8)
                pn.setPen(QPen(fgd));
            else
                pn.setPen(QPen(njfgd));
            pn.drawText(0,0,sz-1,sz-1,Qt::AlignCenter,k);
        }
        return px;
    }
    return QVariant();
}

int QKanjiModel::rowCount(const QModelIndex &) const
{
    return kanjiList.length();
}

void QKanjiModel::createHxBox(QVector<QLine> &rrct, int sz, int hv) const
{
    rrct.clear();
    rrct << QLine(hv,0,sz-1-hv,0)       << QLine(sz-1-hv,0,sz-1,hv);
    rrct << QLine(sz-1,hv,sz-1,sz-1-hv) << QLine(sz-1,sz-1-hv,sz-1-hv,sz-1);
    rrct << QLine(sz-1-hv,sz-1,hv,sz-1) << QLine(hv,sz-1,0,sz-1-hv);
    rrct << QLine(0,sz-1-hv,0,hv)       << QLine(0,hv,hv,0);
}

bool QKanjiModel::isRegularKanji(QChar k)
{
    return (!(k.unicode()>=0x2460 && k.unicode()<=0x24ff)); // exclude unicode enclosed numerics set
}
