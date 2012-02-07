#ifndef KANJIMODEL_H
#define KANJIMODEL_H

#include <QtGui>
#include "kdictionary.h"

class MainWindow;

class QKanjiModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QString kanjiList;
    QFont kanjiFont;
    QHash<QChar,QKanjiInfo> *kanjiInfo;

public:
    explicit QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont,
                         QHash<QChar,QKanjiInfo> *KanjiInfo);
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    int rowCount( const QModelIndex & parent = QModelIndex()) const;
signals:
    
public slots:
    
};

#endif // KANJIMODEL_H
