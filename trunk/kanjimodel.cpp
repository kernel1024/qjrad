#include "kanjimodel.h"
#include "miscutils.h"

QKanjiModel::QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont,
                         QHash<QChar,QKanjiInfo> *KanjiInfo) :
    QAbstractListModel(parent)
{
    kanjiList = KanjiList;
    kanjiFont = KanjiFont;
    kanjiInfo = KanjiInfo;
}

Qt::ItemFlags QKanjiModel::flags(const QModelIndex &) const
{
    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QVariant QKanjiModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (index.row()>=kanjiList.length()) return QVariant();
    if (role == Qt::DecorationRole) {
        QColor bkgd = QApplication::palette("QListView").color(QPalette::Base);
        QColor fgd = QApplication::palette("QListView").color(QPalette::Text);
        QColor njfgd = middleColor(bkgd,fgd,60);

        QChar k = kanjiList.at(index.row());
        QFontMetrics fm(kanjiFont);
        int sz = 12*fm.width(QChar(0x9fa0))/10;
        QPixmap px(sz,sz);
        QPainter pn(&px);
        pn.setFont(kanjiFont);
        pn.fillRect(0,0,sz,sz,bkgd);
        if ((*kanjiInfo)[k].grade<=8)
            pn.setPen(QPen(fgd));
        else
            pn.setPen(QPen(njfgd));
        pn.drawText(0,0,sz,sz,Qt::AlignCenter,k);
        return px;
    }
    return QVariant();
}

int QKanjiModel::rowCount(const QModelIndex &) const
{
    return kanjiList.length();
}
