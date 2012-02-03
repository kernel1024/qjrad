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

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void renderButtons();
    void clearRadButtons();
    void insertOneWidget(QWidget *w, int &row, int &clmn);
private:
    Ui::MainWindow *ui;
    QObjectList buttons;
    bool allowLookup;
public slots:
    void resetRadicals();
    void radicalPressed(bool checked);
};

#endif // MAINWINDOW_H
