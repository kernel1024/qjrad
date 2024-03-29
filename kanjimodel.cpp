#include <QApplication>
#include <QPalette>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>

#include "kanjimodel.h"
#include "kdictionary.h"
#include "global.h"

namespace CDefaults {
const int foundKanjiColorBiasMultiplier = 60;
const int kanjiWidthMultiplier = 12;
const int kanjiWidthDivider = 10;
const int rareKanjiMinimumGrade = 8;
}

ZKanjiModel::ZKanjiModel(QObject *parent, ZKanjiDictionary *dict, const QString &KanjiList)
    : QAbstractListModel(parent)
{
    m_kanjiList = KanjiList;
    m_dict = dict;
}

Qt::ItemFlags ZKanjiModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.row()<m_kanjiList.length()) {
            QChar k = m_kanjiList.at(index.row());
            if (!isRegularKanji(k))
                return Qt::ItemIsEnabled;
        }
    }

    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QVariant ZKanjiModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (index.row()>=m_kanjiList.length()) return QVariant();
    QChar k = m_kanjiList.at(index.row());

    if (role == Qt::DecorationRole) {
        QColor background = QApplication::palette("QListView").color(QPalette::Base);
        QColor kanjiColor = QApplication::palette("QListView").color(QPalette::Text);
        QColor rareKanjiColor = ZGlobal::middleColor(background,kanjiColor,CDefaults::foundKanjiColorBiasMultiplier);

        QFontMetrics fm(zF->fontResults());
        int sz = CDefaults::kanjiWidthMultiplier * fm.horizontalAdvance(CDefaults::biggestRadical)
                 / CDefaults::kanjiWidthDivider;
        QPixmap px(sz,sz);
        QPainter pn(&px);
        pn.setFont(zF->fontResults());
        pn.fillRect(0,0,sz,sz,background);

        if (!isRegularKanji(k)) { // this is a unselectable mark, not kanji
            int v = k.unicode() - CDefaults::enclosedNumericsStart;
            pn.setFont(zF->fontLabels());
            pn.setPen(QPen(rareKanjiColor));
            QVector<QLine> rrct;
            createHxBox(rrct,sz,3);
            pn.drawLines(rrct);
            pn.setPen(QPen(kanjiColor));
            pn.drawText(0,0,sz-1,sz-1,Qt::AlignCenter,tr("%1").arg(v));
        } else { // this is regular kanji
            if (m_dict) {
                if (m_dict->getKanjiGrade(k) <= CDefaults::rareKanjiMinimumGrade) {
                    pn.setPen(QPen(kanjiColor));
                } else {
                    pn.setPen(QPen(rareKanjiColor));
                }
            }
            pn.drawText(0,0,sz-1,sz-1,Qt::AlignCenter,k);
        }
        return px;
    }
    return QVariant();
}

int ZKanjiModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return m_kanjiList.length();
}

void ZKanjiModel::createHxBox(QVector<QLine> &rrct, int sz, int hv) const
{
    rrct.clear();
    rrct << QLine(hv,0,sz-1-hv,0)       << QLine(sz-1-hv,0,sz-1,hv);
    rrct << QLine(sz-1,hv,sz-1,sz-1-hv) << QLine(sz-1,sz-1-hv,sz-1-hv,sz-1);
    rrct << QLine(sz-1-hv,sz-1,hv,sz-1) << QLine(hv,sz-1,0,sz-1-hv);
    rrct << QLine(0,sz-1-hv,0,hv)       << QLine(0,hv,hv,0);
}

bool ZKanjiModel::isRegularKanji(QChar k)
{
    return (!((k.unicode()>=CDefaults::enclosedNumericsStart) &&
              (k.unicode()<=CDefaults::enclosedNumericsEnd))); // exclude unicode enclosed numerics set
}
