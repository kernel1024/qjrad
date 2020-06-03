#include "kanjimodel.h"
#include "miscutils.h"
#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>

namespace CDefaults {
const int foundKanjiColorBiasMultiplier = 60;
const int kanjiWidthMultiplier = 12;
const int kanjiWidthDivider = 10;
const int rareKanjiMinimumGrade = 8;
}

QKanjiModel::QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont,
                         const QFont &MarkFont) :
    QAbstractListModel(parent)
{
    kanjiList = KanjiList;
    kanjiFont = KanjiFont;
    mainWnd = qobject_cast<MainWindow *>(parent);
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
        QColor background = QApplication::palette("QListView").color(QPalette::Base);
        QColor kanjiColor = QApplication::palette("QListView").color(QPalette::Text);
        QColor rareKanjiColor = middleColor(background,kanjiColor,CDefaults::foundKanjiColorBiasMultiplier);

        QFontMetrics fm(kanjiFont);
        int sz = CDefaults::kanjiWidthMultiplier * fm.horizontalAdvance(CDefaults::biggestRadical)
                 / CDefaults::kanjiWidthDivider;
        QPixmap px(sz,sz);
        QPainter pn(&px);
        pn.setFont(kanjiFont);
        pn.fillRect(0,0,sz,sz,background);

        if (!isRegularKanji(k)) { // this is a unselectable mark, not kanji
            int v = k.unicode() - CDefaults::enclosedNumericsStart;
            pn.setFont(markFont);
            pn.setPen(QPen(rareKanjiColor));
            QVector<QLine> rrct;
            createHxBox(rrct,sz,3);
            pn.drawLines(rrct);
            pn.setPen(QPen(kanjiColor));
            pn.drawText(0,0,sz-1,sz-1,Qt::AlignCenter,tr("%1").arg(v));
        } else { // this is regular kanji
            if (mainWnd->getKanjiGrade(k) <= CDefaults::rareKanjiMinimumGrade) {
                pn.setPen(QPen(kanjiColor));
            } else {
                pn.setPen(QPen(rareKanjiColor));
}
            pn.drawText(0,0,sz-1,sz-1,Qt::AlignCenter,k);
        }
        return px;
    }
    return QVariant();
}

int QKanjiModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
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
    return (!((k.unicode()>=CDefaults::enclosedNumericsStart) &&
              (k.unicode()<=CDefaults::enclosedNumericsEnd))); // exclude unicode enclosed numerics set
}
