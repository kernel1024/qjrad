#include <QMessageBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QUrl>
#include <QScrollBar>
#include <QMenu>
#include <QUrlQuery>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include "goldendict/goldendictmgr.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kanjimodel.h"
#include "settingsdlg.h"
#include "miscutils.h"
#include "global.h"
#include "dbusdict.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    if (cgl==NULL)
        cgl = new CGlobal();

    ui->setupUi(this);

    cgl->dbusDict->setMainWindow(this);

    layout()->removeWidget(ui->wdictViewer);
    ui->wdictViewer->setParent(NULL);
    delete ui->wdictViewer;
    ui->wdictViewer=NULL;

    dictView = new QWebEngineView(this);
    dictView->setObjectName(QString::fromUtf8("dictView"));
    dictView->setUrl(QUrl("about://blank"));
    ui->splitterDict->addWidget(dictView);

    QWebEnginePage *wp = new QWebEnginePage(cgl->webProfile,this);
    dictView->setPage(wp);

    connect(cgl->dictManager,&CGoldenDictMgr::showStatusBarMessage,[this](const QString& msg){
        if (msg.isEmpty())
            statusBar()->clearMessage();
        else
            statusBar()->showMessage(msg);
    });

    connect(cgl->dictManager,&CGoldenDictMgr::showCriticalMessage,[this](const QString& msg){
        QMessageBox::critical(this,tr("QJRad - error"),msg);
    });

    infoKanjiTemplate = ui->infoKanji->toHtml();
    ui->infoKanji->clear();

    if (!dict.loadDictionaries()) {
        QMessageBox::critical(this,tr("QJRad - error"),
                              tr("Cannot load main dictionaries\nError: %1").arg(dict.errorString));
    }

    QIcon appicon;
    appicon.addFile(":/data/appicon22.png",QSize(22,22));
    appicon.addFile(":/data/appicon32.png",QSize(32,32));
    appicon.addFile(":/data/appicon48.png",QSize(48,48));
    appicon.addFile(":/data/appicon64.png",QSize(64,64));
    appicon.addFile(":/data/appicon128.png",QSize(128,128));
    setWindowIcon(appicon);

    foundKanji.clear();

    statusMsg = new QLabel(tr("Ready"));
    statusMsg->setMinimumWidth(150);
    statusMsg->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(statusMsg);

    connect(ui->btnReset,SIGNAL(clicked()),this,SLOT(resetRadicals()));
    connect(ui->btnSettings,SIGNAL(clicked()),this,SLOT(settingsDlg()));
    connect(ui->btnOpacity,SIGNAL(clicked()),this,SLOT(opacityList()));
    connect(ui->listKanji,SIGNAL(clicked(QModelIndex)),this,SLOT(kanjiClicked(QModelIndex)));
    connect(ui->listKanji,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(kanjiAdd(QModelIndex)));
    connect(ui->clearScratch,SIGNAL(clicked()),ui->scratchPad,SLOT(clearEditText()));

    connect( ui->scratchPad, SIGNAL(editTextChanged(QString const &)),
             this, SLOT( translateInputChanged( QString const & ) ) );

    connect( ui->scratchPad->lineEdit(), SIGNAL( returnPressed() ),
             this, SLOT( translateInputFinished() ) );

    connect( ui->dictWords, SIGNAL( itemSelectionChanged() ),
             this, SLOT( wordListSelectionChanged() ) );

    connect( cgl->wordFinder, SIGNAL( updated() ),
             this, SLOT( prefixMatchUpdated() ) );
    connect( cgl->wordFinder, SIGNAL( finished() ),
             this, SLOT( prefixMatchFinished() ) );

    connect( dictView, SIGNAL(loadFinished(bool)),this,SLOT(dictLoadFinished(bool)));

    cgl->readSettings();
    ui->scratchPad->setFont(cgl->fontResults);
    ui->dictWords->setFont(cgl->fontBtn);

    allowLookup = false;
    renderButtons();
    allowLookup = true;

    centerWindow();

    cgl->wordFinder->clear();
    cgl->loadDictionaries();

    showEmptyTranslationPage();

    QTimer::singleShot(1000,this,SLOT(updateSplitters()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::centerWindow()
{
    if (cgl->geomFirstWinPos) {
        move(cgl->savedWinPos);
        resize(cgl->savedWinSize);
        cgl->geomFirstWinPos = false;
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

    ui->splitterMain->setCollapsible(1,true);
    widths.clear();
    widths << width() - cgl->savedDictSplitterPos;
    widths << cgl->savedDictSplitterPos;
    ui->splitterMain->setSizes(widths);

    widths.clear();
    widths << 2*height()/5;
    widths << 3*height()/5;
    ui->splitterDict->setSizes(widths);
}

void MainWindow::clearRadButtons()
{
    while (!buttons.isEmpty())
        buttons.takeFirst()->deleteLater();
}

QList<int> MainWindow::getSplittersSize()
{
    return ui->splitter->sizes();
}

QList<int> MainWindow::getDictSplittersSize()
{
    return ui->splitterMain->sizes();
}

int MainWindow::getKanjiGrade(const QChar &kanji)
{
    if (dict.kanjiGrade.contains(kanji))
        return dict.kanjiGrade.value(kanji);
    else
        return 0;
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
            rl->setFont(cgl->fontLabels);
            rl->setFrameShape(QFrame::Box);
            w = rl;
            rmark=ri.strokes;
        }
        insertOneWidget(w,row,clmn);
        // insert button
        QPushButton *pb = new QPushButton(ri.radical,ui->frameRad);
        pb->setFlat(true);
        pb->setFont(cgl->fontBtn);
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
        if (clmn>=cgl->maxHButtons) {
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

void MainWindow::radicalPressed(const bool)
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
        // insert unselectable labels between kanji groups with different stroke count
        int idx = 0;
        int prevsc = 0;
        while (idx<foundKanji.length()) {
            if (prevsc!=dict.kanjiStrokes[foundKanji.at(idx)]) {
                prevsc = dict.kanjiStrokes[foundKanji.at(idx)];
                foundKanji.insert(idx,QChar(0x2460+prevsc)); // use enclosed numerics set from unicode
                idx++;
            }
            idx++;
        }
    } else {
        foundKanji.clear();
    }

    QItemSelectionModel *m = ui->listKanji->selectionModel();
    QAbstractItemModel *n = ui->listKanji->model();
    ui->listKanji->setModel(new QKanjiModel(this,foundKanji,cgl->fontResults,cgl->fontLabels));
    m->deleteLater();
    n->deleteLater();
    ui->infoKanji->clear();

    if (bpcnt>0)
        statusMsg->setText(tr("Found %1 kanji").arg(foundKanji.length()));
    else
        statusMsg->setText(tr("Ready"));

    ui->listKanji->verticalScrollBar()->setSingleStep(ui->listKanji->verticalScrollBar()->pageStep());
}

void MainWindow::kanjiClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    if (index.row()>=foundKanji.length()) return;
    QChar k = foundKanji.at(index.row());
    if (!QKanjiModel::isRegularKanji(k)) return;
    ui->infoKanji->clear();
    QKanjiInfo ki = dict.getKanjiInfo(k);
    if (ki.isEmpty()) {
        ui->infoKanji->setText(tr("Kanji %1 not found in dictionary.").arg(k));
        return;
    }
    QString msg;
    int strokes = dict.kanjiStrokes[k];
    int grade = dict.kanjiGrade[k];

    msg = QString(infoKanjiTemplate)
          .arg(cgl->fontResults.family())
          .arg(ki.kanji)
          .arg(strokes)
          .arg(ki.parts.join(tr(" ")))
          .arg(grade)
          .arg(ki.onReading.join(tr(", ")))
          .arg(ki.kunReading.join(tr(", ")))
          .arg(ki.meaning.join(tr(", ")));

    ui->infoKanji->setHtml(msg);
}

void MainWindow::kanjiAdd(const QModelIndex &index)
{
    if (!index.isValid()) return;
    if (index.row()>=foundKanji.length()) return;
    QChar k = foundKanji.at(index.row());
    if (!QKanjiModel::isRegularKanji(k)) return;
    ui->scratchPad->setEditText(ui->scratchPad->currentText()+k);
}

void MainWindow::setScratchPadText(const QString &text)
{
    ui->scratchPad->setEditText(text);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    cgl->writeSettings(this);
    event->accept();
}

void MainWindow::settingsDlg()
{
    QSettingsDlg *dlg = new QSettingsDlg(this,cgl->fontBtn,cgl->fontLabels,cgl->fontResults,cgl->maxHButtons,
                                         cgl->getDictPaths());
    if (dlg->exec()) {
        cgl->fontBtn = dlg->fontBtn;
        cgl->fontLabels = dlg->fontLabels;
        cgl->fontResults = dlg->fontResults;
        cgl->maxHButtons = dlg->maxHButtons;
        allowLookup = false;
        renderButtons();
        allowLookup = true;
        resetRadicals();
        ui->scratchPad->setFont(cgl->fontResults);
        ui->dictWords->setFont(cgl->fontBtn);
        cgl->setDictPaths(dlg->getDictPaths());
        cgl->wordFinder->clear();
        cgl->loadDictionaries();
    }
    dlg->setParent(NULL);
    delete dlg;
}

void MainWindow::opacityList()
{
    QAction* ac = qobject_cast<QAction* >(sender());
    if (ac!=NULL) {
        int op = ac->data().toInt();
        if (op>0 && op<=100)
            setWindowOpacity((qreal)op/100.0);
        return;
    }

    QMenu* m = new QMenu();
    for (int i=0;i<=10;i++) {
        QAction* ac = new QAction(QString("%1%").arg(50+i*5),NULL);
        connect(ac,SIGNAL(triggered()),this,SLOT(opacityList()));
        ac->setData(50+i*5);
        m->addAction(ac);
    }

    m->setAttribute(Qt::WA_DeleteOnClose,true);
    m->popup(QCursor::pos());
}

void MainWindow::wordListItemActivated( QListWidgetItem * item )
{
    QString newValue = item->text();

    if ((ui->scratchPad->findText(newValue)<0) && !newValue.isEmpty())
        ui->scratchPad->addItem(newValue);

    showTranslationFor( newValue );
}

void MainWindow::wordListSelectionChanged()
{
    QList< QListWidgetItem * > selected = ui->dictWords->selectedItems();

    if ( selected.size() )
        wordListItemActivated( selected.front() );
}

void MainWindow::dictLoadFinished(bool)
{
    dictView->unsetCursor();
}

void MainWindow::prefixMatchUpdated()
{
    updateMatchResults( false );
}

void MainWindow::prefixMatchFinished()
{
    updateMatchResults( true );
}

void MainWindow::updateMatchResults(bool finished)
{
    WordFinder::SearchResults const & results = cgl->wordFinder->getResults();

    ui->dictWords->setUpdatesEnabled( false );

    for( unsigned x = 0; x < results.size(); ++x )
    {
        QListWidgetItem * i = ui->dictWords->item( x );

        if ( !i )
        {
            i = new QListWidgetItem( results[ x ].first, ui->dictWords );

            if ( results[ x ].second )
            {
                QFont f = i->font();
                f.setItalic( true );
                i->setFont( f );
            }
            ui->dictWords->addItem( i );
        }
        else
        {
            if ( i->text() != results[ x ].first )
                i->setText( results[ x ].first );

            QFont f = i->font();
            if ( f.italic() != results[ x ].second )
            {
                f.setItalic( results[ x ].second );
                i->setFont( f );
            }
        }
        if (i->text().at(0).direction() == QChar::DirR)
            i->setTextAlignment(Qt::AlignRight);
        if (i->text().at(0).direction() == QChar::DirL)
            i->setTextAlignment(Qt::AlignLeft);
    }

    while ( ui->dictWords->count() > (int) results.size() )
    {
        // Chop off any extra items that were there
        QListWidgetItem * i = ui->dictWords->takeItem( ui->dictWords->count() - 1 );

        if ( i )
            delete i;
        else
            break;
    }

    if ( ui->dictWords->count() )
    {
        ui->dictWords->scrollToItem( ui->dictWords->item( 0 ), QAbstractItemView::PositionAtTop );
        ui->dictWords->setCurrentItem( 0, QItemSelectionModel::Clear );
    }

    ui->dictWords->setUpdatesEnabled( true );

    if ( finished )
    {
        ui->dictWords->unsetCursor();

        if ( !cgl->wordFinder->getErrorString().isEmpty() )
            statusBar()->showMessage( tr( "WARNING: %1" ).arg(cgl->wordFinder->getErrorString() ) );
    }
}

void MainWindow::translateInputChanged( QString const & newValue )
{
    if ((ui->scratchPad->findText(newValue)<0) && !newValue.isEmpty())
        ui->scratchPad->addItem(ui->scratchPad->currentText());

    showEmptyTranslationPage();

    // If there's some status bar message present, clear it since it may be
    // about the previous search that has failed.
    if ( !statusBar()->currentMessage().isEmpty() )
        statusBar()->clearMessage();

    // If some word is selected in the word list, unselect it. This prevents
    // triggering a set of spurious activation signals when the list changes.

    if ( ui->dictWords->selectionModel()->hasSelection() )
        ui->dictWords->setCurrentItem( 0, QItemSelectionModel::Clear );

    QString req = newValue.trimmed();

    if ( !req.size() )
    {
        // An empty request always results in an empty result
        cgl->wordFinder->cancel();
        ui->dictWords->clear();
        ui->dictWords->unsetCursor();

        return;
    }

    ui->dictWords->setCursor( Qt::WaitCursor );

    cgl->wordFinder->prefixMatch( req, cgl->dictManager->dictionaries );
}

void MainWindow::translateInputFinished()
{
    QString word = ui->scratchPad->currentText();

    if ( word.size() )
        showTranslationFor( word );
}

void MainWindow::showTranslationFor( QString const & inWord )
{
    QUrl req;
    req.setScheme( "gdlookup" );
    req.setHost( "localhost" );
    QUrlQuery requ;
    requ.addQueryItem( "word", inWord );
    req.setQuery(requ);
    dictView->load( req );

    dictView->setCursor( Qt::WaitCursor );
}

void MainWindow::showEmptyTranslationPage()
{
    QUrl req;

    req.setScheme( "gdlookup" );
    req.setHost( "localhost" );
    QUrlQuery requ;
    requ.addQueryItem( "blank", "1" );
    req.setQuery(requ);

    dictView->load( req );

    dictView->setCursor( Qt::WaitCursor );
}
