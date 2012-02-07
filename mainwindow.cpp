#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kanjimodel.h"
#include "settingsdlg.h"
#include "miscutils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if (!dict.loadDictionaries()) {
        QMessageBox::critical(this,tr("QJRad - error"),tr("Cannot load main dictionaries\nError: %1").arg(dict.errorString));
        return;
    }

    QIcon appicon;
    appicon.addFile(":/data/appicon22.png",QSize(22,22));
    appicon.addFile(":/data/appicon32.png",QSize(32,32));
    appicon.addFile(":/data/appicon48.png",QSize(48,48));
    appicon.addFile(":/data/appicon64.png",QSize(64,64));
    appicon.addFile(":/data/appicon128.png",QSize(128,128));
    setWindowIcon(appicon);

    foundKanji.clear();
    geomFirstWinPos = false;
    savedSplitterPos = 200;

    statusMsg = new QLabel(tr("Ready"));
    statusMsg->setMinimumWidth(150);
    statusMsg->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(statusMsg);

    connect(ui->btnReset,SIGNAL(clicked()),this,SLOT(resetRadicals()));
    connect(ui->btnSettings,SIGNAL(clicked()),this,SLOT(settingsDlg()));
    connect(ui->listKanji,SIGNAL(clicked(QModelIndex)),this,SLOT(kanjiClicked(QModelIndex)));
    connect(ui->listKanji,SIGNAL(activated(QModelIndex)),this,SLOT(kanjiAdd(QModelIndex)));

    readSettings();

    allowLookup = false;
    renderButtons();
    allowLookup = true;

    centerWindow();
    updateSplitters();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::centerWindow()
{
    if (geomFirstWinPos) {
        move(savedWinPos);
        resize(savedWinSize);
        geomFirstWinPos = false;
        return;
    }
    int screen = 0;
    QWidget *w = window();
    QDesktopWidget *desktop = QApplication::desktop();
    if (w) {
        screen = desktop->screenNumber(w);
    } else if (desktop->isVirtualDesktop()) {
        screen = desktop->screenNumber(QCursor::pos());
    } else {
        screen = desktop->screenNumber(this);
    }
    QRect rect(desktop->availableGeometry(screen));
    int h = 60*rect.height()/100;
    if (h<650) h = qMin(650,75*rect.height()/100);
    QSize nw(w->height(),h);
    resize(nw);
    move(rect.width()/2 - frameGeometry().width()/2,
         rect.height()/2 - frameGeometry().height()/2);
}

void MainWindow::updateSplitters()
{
    ui->splitter->setCollapsible(0,true);

    QList<int> widths;
    widths.clear();
    widths << width()-savedSplitterPos;
    widths << savedSplitterPos;
    ui->splitter->setSizes(widths);
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
    int rmark=0, row=0, clmn=0;

    QWidget *w;
    for (int i=0;i<dict.radicalsLookup.count();i++) {
        QKRadItem ri = dict.radicalsLookup.at(i);
        w = NULL;
        if (rmark!=ri.strokes) {
            // insert label
            QLabel *rl = new QLabel(tr("%1").arg(ri.strokes),ui->frameRad);
            rl->setAlignment(Qt::AlignCenter);
            rl->setFont(fontLabels);
            rl->setFrameShape(QFrame::Box);
            w = rl;
            rmark=ri.strokes;
        }
        insertOneWidget(w,row,clmn);
        // insert button
        QPushButton *pb = new QPushButton(ri.radical,ui->frameRad);
        pb->setFlat(true);
        pb->setFont(fontBtn);
        pb->setCheckable(true);
        // qt 4.8 bug with kde color configuration tool. disabled color is still incorrect, use our specific color
        QPalette p = pb->palette();
        p.setBrush(QPalette::Disabled,QPalette::ButtonText,QBrush(
                       middleColor(QApplication::palette("QPushButton").color(QPalette::Button),
                                   QApplication::palette("QPushButton").color(QPalette::ButtonText),25)));
        pb->setPalette(p);
        // ----
        if (btnWidth<0) {
            QFontMetrics fm(pb->font());
            btnWidth = 13*fm.width(QChar(0x9fa0))/10;
        }
        pb->setMinimumWidth(btnWidth);
        pb->setMinimumHeight(btnWidth);
        pb->setMaximumHeight(btnWidth+2);
        connect(pb,SIGNAL(clicked(bool)),this,SLOT(radicalPressed(bool)));
        w = pb;
        insertOneWidget(w,row,clmn);
    }

}

void MainWindow::insertOneWidget(QWidget *w, int &row, int &clmn)
{
    if (w!=NULL) {
        ui->gridRad->addWidget(w,row,clmn);
        buttons << w;
        clmn++;
        if (clmn>=maxHButtons) {
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
        pb->setEnabled(true);
    }
    allowLookup = true;
    radicalPressed(false);
    statusMsg->setText(tr("Ready"));
}

void MainWindow::radicalPressed(bool)
{
    if (!allowLookup) return;

    QList<QPushButton *> pbl;
    pbl.clear();
    for (int i=0;i<buttons.count();i++)
        if (qobject_cast<QPushButton *>(buttons.at(i))!=NULL)
            pbl << qobject_cast<QPushButton *>(buttons.at(i));

    QStringList kl;
    kl.clear();
    int bpcnt = 0;
    for (int i=0;i<pbl.count();i++) {
        QPushButton *pb = pbl.at(i);
        QChar r = pb->text().at(0);
        pb->setEnabled(true);
        if (pb->isChecked()) {
            bpcnt++;
            if (!r.isNull()) {
                int idx = dict.radicalsLookup.indexOf(QKRadItem(r));
                if (idx>=0) {
                    kl << dict.radicalsLookup.at(idx).kanji;
                }
            }
        }
    }
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

    if (!kl.isEmpty()) {
        // sort kanji by radicals weight and by unicode weight
        foundKanji = dict.sortKanji(kl.takeFirst());
        // disable radicals that not appears on found set entirely
        QList<QChar> ards;
        ards.clear();
        for (int i=0;i<foundKanji.length();i++) {
            QChar kj = foundKanji.at(i);
            if (dict.kanjiParts.contains(kj)) {
                QString krad = dict.kanjiParts[kj].join("");
                for (int j=0;j<krad.length();j++) {
                    if (!ards.contains(krad.at(j)))
                        ards << krad.at(j);
                }
            }
        }
        for (int i=0;i<pbl.count();i++) {
            if (!ards.contains(pbl.at(i)->text().at(0)))
                pbl.at(i)->setEnabled(false);
        }
    } else {
        foundKanji.clear();
    }

    QItemSelectionModel *m = ui->listKanji->selectionModel();
    QAbstractItemModel *n = ui->listKanji->model();
    ui->listKanji->setModel(new QKanjiModel(this,foundKanji,fontResults, &(dict.kanjiInfo)));
    m->deleteLater();
    n->deleteLater();
    ui->infoKanji->clear();

    if (bpcnt>0)
        statusMsg->setText(tr("Found %1 kanji").arg(foundKanji.length()));
    else
        statusMsg->setText(tr("Ready"));
}

void MainWindow::kanjiClicked(const QModelIndex &index)
{
    ui->infoKanji->clear();
    if (!index.isValid()) return;
    if (index.row()>=foundKanji.length()) return;
    QChar k = foundKanji.at(index.row());
    if (!dict.kanjiInfo.contains(k)) {
        ui->infoKanji->setText(tr("Kanji %1 not found in dictionary.").arg(k));
        return;
    }
    QString msg;
    QKanjiInfo ki = dict.kanjiInfo[k];
    msg = tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">");
    msg += tr("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">");
    msg += tr("p, li { white-space: pre-wrap; }");
    msg += tr("</style></head><body style=\" font-family:'%1'; font-size:%2pt; font-weight:400; font-style:normal;\">").
            arg(QApplication::font("QTextBrowser").family()).
            arg(QApplication::font("QTextBrowser").pointSize());
    msg += tr("<p style=\" margin-top:0px; margin-bottom:10px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'%1'; font-size:36pt;\">%2</span></p>").arg(fontResults.family()).arg(ki.kanji);

    msg += tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Strokes:</span> %1</p>").arg(ki.strokes);
    msg += tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Parts:</span> %1</p>").arg(ki.parts.join(tr(" ")));
    msg += tr("<p style=\" margin-top:0px; margin-bottom:10px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Grade:</span> %1</p>").arg(ki.grade);

    msg += tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">On:</span> %1</p>").arg(ki.onReading.join(tr(", ")));
    msg += tr("<p style=\" margin-top:0px; margin-bottom:10px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Kun:</span> %1</p>").arg(ki.kunReading.join(tr(", ")));

    msg += tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">Meaning:</span> %1</p>").arg(ki.meaning.join(tr(", ")));
    msg += tr("</body></html>");

    ui->infoKanji->setHtml(msg);
}

void MainWindow::kanjiAdd(const QModelIndex &index)
{
    if (!index.isValid()) return;
    if (index.row()>=foundKanji.length()) return;
    QChar k = foundKanji.at(index.row());
    ui->scratchPad->setText(ui->scratchPad->text()+k);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    writeSettings();
    event->accept();
}

void MainWindow::settingsDlg()
{
    QSettingsDlg *dlg = new QSettingsDlg(this,fontBtn,fontLabels,fontResults,maxHButtons);
    if (dlg->exec()) {
        fontBtn = dlg->fontBtn;
        fontLabels = dlg->fontLabels;
        fontResults = dlg->fontResults;
        maxHButtons = dlg->maxHButtons;
        allowLookup = false;
        renderButtons();
        allowLookup = true;
        resetRadicals();
        ui->scratchPad->setFont(fontResults);
    }
    dlg->setParent(NULL);
    delete dlg;
}

void MainWindow::readSettings()
{
    QFont fontResL = QApplication::font("QListView");
    fontResL.setPointSize(14);
    QFont fontBtnL = QApplication::font("QPushButton");
    fontBtnL.setPointSize(14);
    QFont fontBtnLabelL = QApplication::font("QLabel");
    fontBtnLabelL.setPointSize(12);
    fontBtnLabelL.setWeight(QFont::Bold);

    QSettings se("kilobax","qjrad");
    se.beginGroup("Main");
    fontResults = qvariant_cast<QFont>(se.value("fontResult",fontResL));
    fontBtn = qvariant_cast<QFont>(se.value("fontButton",fontBtnL));
    fontLabels = qvariant_cast<QFont>(se.value("fontLabel",fontBtnLabelL));
    maxHButtons = se.value("maxHButtons",30).toInt();
    se.endGroup();
    se.beginGroup("Geometry");
    savedWinPos = se.value("winPos",QPoint(20,20)).toPoint();
    savedWinSize = se.value("winSize",QSize(200,200)).toSize();
    geomFirstWinPos = true;
    bool okconv;
    savedSplitterPos = se.value("splitterPos",200).toInt(&okconv);
    if (!okconv) savedSplitterPos = 200;
    se.endGroup();

    ui->scratchPad->setFont(fontResults);
}

void MainWindow::writeSettings()
{
    QSettings se("kilobax","qjrad");
    se.beginGroup("Main");
    se.setValue("fontResult",fontResults);
    se.setValue("fontButton",fontBtn);
    se.setValue("fontLabel",fontLabels);
    se.setValue("maxHButtons",maxHButtons);
    se.endGroup();
    se.beginGroup("Geometry");
    se.setValue("winPos",pos());
    se.setValue("winSize",size());
    QList<int> szs = ui->splitter->sizes();
    int ssz = 200;
    if (szs.count()>=2) ssz = szs[1];
    se.setValue("splitterPos",ssz);
    se.endGroup();
}
