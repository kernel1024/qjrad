#include "kanjimodel.h"

QKanjiModel::QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont) :
    QAbstractListModel(parent)
{
    kanjiList = KanjiList;
    kanjiFont = KanjiFont;
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
        QFontMetrics fm(kanjiFont);
        int sz = 12*fm.width(QChar(0x9fa0))/10;
        QPixmap px(sz,sz);
        QPainter pn(&px);
        pn.setFont(kanjiFont);
        pn.fillRect(0,0,sz,sz,Qt::white);
        pn.drawText(0,0,sz,sz,Qt::AlignCenter,kanjiList.at(index.row()));
        return px;
    }
    return QVariant();
}

int QKanjiModel::rowCount(const QModelIndex &) const
{
    return kanjiList.length();
}
