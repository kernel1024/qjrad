#include <QApplication>
#include "mainwindow.h"
#include "kdictionary.h"
#include "global.h"

#ifdef WITH_OCR
tesseract::TessBaseAPI* ocr = NULL;
#endif

int main(int argc, char *argv[])
{
    setlocale (LC_NUMERIC, "C");
#ifdef WITH_OCR
    ocr = initializeOCR();
#endif

    qRegisterMetaType<QKRadItem>("QKRadItem");
    qRegisterMetaType<QKanjiInfo>("QKanjiInfo");

    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    return a.exec();
}
