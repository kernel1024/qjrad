#ifndef KANJIMODEL_H
#define KANJIMODEL_H

#include <QAbstractListModel>
#include <QPointer>

namespace CDefaults {
const QChar biggestRadical(0x9fa0); // yaku/fue, biggest radical
const int enclosedNumericsStart = 0x2460;
const int enclosedNumericsEnd = 0x24ff;
}

class ZKanjiDictionary;

class ZKanjiModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QString m_kanjiList;
    QPointer<ZKanjiDictionary> m_dict;

    void createHxBox(QVector<QLine> &rrct, int sz, int hv = 2) const;

public:
    ZKanjiModel(QObject *parent, ZKanjiDictionary *dict, const QString &KanjiList);
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    int rowCount( const QModelIndex & parent = QModelIndex()) const;
    static bool isRegularKanji(QChar k);

};

#endif // KANJIMODEL_H
