#ifndef KANJIMODEL_H
#define KANJIMODEL_H

#include <QtGui>
#include "mainwindow.h"

class MainWindow;

class QKanjiModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QString kanjiList;
    QFont kanjiFont, markFont;
    MainWindow* mainWnd;

    void createHxBox(QVector<QLine> &rrct, int sz, int hv = 2) const;

public:
    explicit QKanjiModel(QObject *parent, const QString &KanjiList, const QFont &KanjiFont,
                         const QFont &MarkFont);
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    int rowCount( const QModelIndex & parent = QModelIndex()) const;
    static bool isRegularKanji(QChar k);
signals:
    
public slots:
    
};

#endif // KANJIMODEL_H
