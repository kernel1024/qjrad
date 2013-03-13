#include <QApplication>
#include "mainwindow.h"
#include "kdictionary.h"
#include "global.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<QKRadItem>("QKRadItem");
    qRegisterMetaType<QKanjiInfo>("QKanjiInfo");

    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    return a.exec();
}
