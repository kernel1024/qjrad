#include <QApplication>
#include "mainwindow.h"
#include "kdictionary.h"
#include "global.h"

#ifdef WITH_OCR
tesseract::TessBaseAPI* ocr = nullptr;
#endif

int main(int argc, char *argv[])
{
    setlocale (LC_NUMERIC, "C");
#ifdef JTESS_API4
    setlocale (LC_ALL, "C");
    setlocale (LC_CTYPE, "C");
#endif
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
