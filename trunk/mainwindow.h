#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include "kdictionary.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QKDictionary dict;
    QFont fontResults, fontBtn, fontLabels;
    QString foundKanji;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void renderButtons();
    void clearRadButtons();
private:
    Ui::MainWindow *ui;
    QObjectList buttons;
    bool allowLookup;
    QLabel *statusMsg;
    int maxHButtons;
    void centerWindow();
    void insertOneWidget(QWidget *w, int &row, int &clmn);
    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent * event);
    void updateSplitters();

public slots:
    void resetRadicals();
    void radicalPressed(bool checked);
    void settingsDlg();
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);
};

#endif // MAINWINDOW_H
