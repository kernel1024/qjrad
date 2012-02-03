#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if (!dict.loadDictionaries()) {
        QMessageBox::critical(this,tr("QJRad - error"),tr("Cannot load main dictionaries"));
        close();
        return;
    }

    connect(ui->btnReset,SIGNAL(clicked()),this,SLOT(resetRadicals()));

    allowLookup = false;
    renderButtons();
    allowLookup = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearRadButtons()
{
    while (!buttons.isEmpty())
        buttons.takeFirst()->deleteLater();
}

void MainWindow::renderButtons()
{
    int btnWidth = -1;

    clearRadButtons();

    ui->gridRad->setHorizontalSpacing(2);
    ui->gridRad->setVerticalSpacing(2);
    QFont bf = QApplication::font("QPushButton");
    bf.setPointSize(14);
    int rmark=0, row=0, clmn=0;
    QFont bf2 = QApplication::font("QLabel");
    bf2.setPointSize(12);
    bf2.setWeight(QFont::Bold);

    QWidget *w;
    for (int i=0;i<dict.radicalsLookup.count();i++) {
        QKRadItem ri = dict.radicalsLookup.at(i);
        w = NULL;
        if (rmark!=ri.strokes) { // insert label
            QLabel *rl = new QLabel(tr("%1").arg(ri.strokes),ui->frameRad);
            rl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            rl->setFont(bf2);
            rl->setFrameShape(QFrame::Box);
            w = rl;
            rmark=ri.strokes;
        }
        insertOneWidget(w,row,clmn);
        // insert button
        QPushButton *pb = new QPushButton(ri.radical,ui->frameRad);
        pb->setFlat(true);
        pb->setFont(bf);
        pb->setCheckable(true);
        if (btnWidth<0) {
            QFontMetrics fm(pb->font());
            btnWidth = fm.width(QChar(0x9fa0)) + 15;
        }
        pb->setMinimumWidth(btnWidth);
        connect(pb,SIGNAL(clicked(bool)),this,SLOT(radicalPressed(bool)));
        w = pb;
        insertOneWidget(w,row,clmn);
    }

}

void MainWindow::insertOneWidget(QWidget *w, int &row, int &clmn)
{
    const int maxClmn = 30;
    if (w!=NULL) {
        ui->gridRad->addWidget(w,row,clmn);
        buttons << w;
        clmn++;
        if (clmn>=maxClmn) {
            clmn=0;
            row++;
        }
    }
}

void MainWindow::resetRadicals()
{
    allowLookup = false;
    for (int i=0;i<buttons.count();i++) {
        QPushButton *pb = qobject_cast<QPushButton *>(buttons.at(i));
        if (pb==NULL) continue;
        pb->setChecked(false);
    }
    allowLookup = true;
}

void MainWindow::radicalPressed(bool)
{
    QStringList kl;
    kl.clear();
    for (int i=0;i<buttons.count();i++) {
        QPushButton *pb = qobject_cast<QPushButton *>(buttons.at(i));
        if (pb==NULL) continue;
        if (pb->isChecked()) {
            QChar r = pb->text().at(0);
            if (!r.isNull()) {
                int idx = dict.radicalsLookup.indexOf(QKRadItem(r));
                if (idx>=0) {
                    kl << dict.radicalsLookup.at(idx).kanji;
                }
            }
        }
    }
    if (kl.isEmpty()) return;
    while (kl.count()>1) {
        QString fs = kl.at(0);
        QString ms = kl.takeLast();
        int i=0;
        while (i<fs.length()) {
            if (!ms.contains(fs.at(i)))
                fs = fs.remove(i,1);
            else
                i++;
        }
        if (fs.isEmpty()) {
            kl.clear();
            kl << QString();
            break;
        }
        kl.replace(0,fs);
    }
    qDebug() << kl.takeFirst();


 }
