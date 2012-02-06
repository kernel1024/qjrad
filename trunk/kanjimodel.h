#ifndef KANJIMODEL_H
#define KANJIMODEL_H

#include <QtGui>

class MainWindow;

class QKanjiModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QString kanjiList;
    QFont kanjiFont;

public:
    explicit QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont);
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    int rowCount( const QModelIndex & parent = QModelIndex()) const;
signals:
    
public slots:
    
};

#endif // KANJIMODEL_H