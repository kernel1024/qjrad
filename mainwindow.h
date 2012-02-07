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
    // geometry restore
    bool geomFirstWinPos;
    QPoint savedWinPos;
    QSize savedWinSize;
    int savedSplitterPos;
    // ---
    void insertOneWidget(QWidget *w, int &row, int &clmn);
    void closeEvent(QCloseEvent * event);

public slots:
    // window geometry and misc
    void centerWindow();
    void readSettings();
    void writeSettings();
    void updateSplitters();
    // event handlers
    void resetRadicals();
    void radicalPressed(bool checked);
    void settingsDlg();
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);
};

#endif // MAINWINDOW_H
