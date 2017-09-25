#include <QMessageBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QUrl>
#include <QScrollBar>
#include <QMenu>
#include <QUrlQuery>
#include <goldendictlib/goldendictmgr.hh>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kanjimodel.h"
#include "settingsdlg.h"
#include "miscutils.h"
#include "global.h"
#include "dbusdict.h"
#include "regiongrabber.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    if (cgl==nullptr)
        cgl = new CGlobal();

    ui->setupUi(this);

    cgl->dbusDict->setMainWindow(this);

    forceFocusToEdit = false;
    dictView = ui->wdictViewer;

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

    connect(ui->btnReset,&QPushButton::clicked,this,&MainWindow::resetRadicals);
    connect(ui->btnSettings,&QPushButton::clicked,this,&MainWindow::settingsDlg);
    connect(ui->btnOpacity,&QPushButton::clicked,this,&MainWindow::opacityList);
    connect(ui->listKanji,&QListView::clicked,this,&MainWindow::kanjiClicked);
    connect(ui->listKanji,&QListView::doubleClicked,this,&MainWindow::kanjiAdd);
    connect(ui->clearScratch,&QPushButton::clicked,ui->scratchPad,&QComboBox::clearEditText);
    connect(ui->btnCapture,&QPushButton::clicked,this,&MainWindow::screenCapture);

    connect(ui->btnBackspace,&QPushButton::clicked,[this](bool){
        if (!ui->scratchPad->currentText().isEmpty())
            ui->scratchPad->setEditText(ui->scratchPad->currentText().left(
                                            ui->scratchPad->currentText().length()-1));
    });

    connect(ui->scratchPad,&QComboBox::editTextChanged,this,&MainWindow::translateInputChanged);
    connect(ui->scratchPad->lineEdit(),&QLineEdit::returnPressed,this,&MainWindow::translateInputFinished);
    connect(ui->dictWords,&QListWidget::itemSelectionChanged,this,&MainWindow::wordListSelectionChanged);
    connect(ui->dictWords,&QListWidget::itemDoubleClicked,this,&MainWindow::wordListLookupItem);
    connect(cgl->wordFinder,&WordFinder::updated,this,&MainWindow::prefixMatchUpdated);
    connect(cgl->wordFinder,&WordFinder::finished,this,&MainWindow::prefixMatchFinished);

    connect(dictView, &QTextBrowser::textChanged,this,&MainWindow::dictLoadFinished);

    keyFilter = new CAuxDictKeyFilter(this);
    ui->scratchPad->installEventFilter(keyFilter);
    connect(keyFilter, &CAuxDictKeyFilter::keyPressed, [this](int){
        forceFocusToEdit = true;
    });

    connect(ui->radioHiragana,&QRadioButton::clicked, this, &MainWindow::updateKana);
    connect(ui->radioKatakana,&QRadioButton::clicked, this, &MainWindow::updateKana);

    cgl->readSettings();
    ui->scratchPad->setFont(cgl->fontResults);
    ui->dictWords->setFont(cgl->fontBtn);

    allowLookup = false;
    renderRadicalsButtons();
    renderKanaButtons();
    allowLookup = true;

    centerWindow();

    cgl->wordFinder->clear();
    cgl->loadDictionaries();

    showEmptyTranslationPage();

    QTimer::singleShot(1000,this,&MainWindow::updateSplitters);
#ifndef WITH_OCR
    ui->btnCapture->setEnabled(false);
    ui->btnCapture->hide();
#endif
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

void MainWindow::clearKanaButtons()
{
    while (!kanaButtons.isEmpty())
        kanaButtons.takeFirst()->deleteLater();
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

void MainWindow::renderRadicalsButtons()
{
    int btnWidth = -1;

    clearRadButtons();

    ui->gridRad->setHorizontalSpacing(2);
    ui->gridRad->setVerticalSpacing(2);
    int rmark=0, row=0, clmn=0;

    QWidget *w;
    for (int i=0;i<dict.radicalsLookup.count();i++) {
        QKRadItem ri = dict.radicalsLookup.at(i);
        w = nullptr;
        if (rmark!=ri.strokes) {
            // insert label
            QLabel *rl = new QLabel(tr("%1").arg(ri.strokes),ui->frameRad);
            rl->setAlignment(Qt::AlignCenter);
            rl->setFont(cgl->fontLabels);
            rl->setFrameShape(QFrame::Box);
            w = rl;
            rmark=ri.strokes;
            insertOneWidget(w,row,clmn,false);
        }
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
        connect(pb,&QPushButton::clicked,this,&MainWindow::radicalPressed);
        w = pb;
        insertOneWidget(w,row,clmn,false);
    }
}

void MainWindow::renderKanaButtons()
{
    QList<int> kana = {
        0x3042, 0x3044, 0x3046, 0x3048, 0x304A,
        0x3041, 0x3043, 0x3045, 0x3047, 0x3049,
        0x304b, 0x304d, 0x304f, 0x3051, 0x3053,
        0x304c, 0x304e, 0x3050, 0x3052, 0x3054,
        0x3055, 0x3057, 0x3059, 0x305b, 0x305d,
        0x3056, 0x3058, 0x305a, 0x305c, 0x305e,
        0x305f, 0x3061, 0x3064, 0x3066, 0x3068,
        0x3060, 0x3062, 0x3065, 0x3067, 0x3069,
        0x306a, 0x306b, 0x306c, 0x306d, 0x306e,
        0x306f, 0x3072, 0x3075, 0x3078, 0x307b,
        0x3070, 0x3073, 0x3076, 0x3079, 0x307c,
        0x3071, 0x3074, 0x3077, 0x307a, 0x307d,
        0x307e, 0x307f, 0x3080, 0x3081, 0x3082,
        0x3089, 0x308a, 0x308b, 0x308c, 0x308d,
        0x308f, 0x3090, 0x3091, 0x3092, 0x3093,
        0x3084, 0x3086, 0x3088, 0x3094, 0x3095,
        0x3083, 0x3085, 0x3087, 0x3063, 0x3096
    };
    int btnWidth = -1;

    clearKanaButtons();

    ui->gridKana->setHorizontalSpacing(2);
    ui->gridKana->setVerticalSpacing(2);
    int row=0, clmn=0;

    QWidget *w;
    foreach (const int i, kana) {
        // insert button
        QChar k = QChar(i);
        if (ui->radioKatakana->isChecked())
            k = QChar(i+(0x30A0-0x3040));
        QPushButton *pb = new QPushButton(k,ui->frameKana);
        pb->setFlat(true);
        pb->setFont(cgl->fontBtn);
        if (btnWidth<0) {
            QFontMetrics fm(pb->font());
            btnWidth = 13*fm.width(QChar(0x9fa0))/10;
        }
        pb->setMinimumWidth(btnWidth);
        pb->setMinimumHeight(btnWidth);
        pb->setMaximumHeight(btnWidth+2);
        connect(pb,&QPushButton::clicked,this,&MainWindow::kanaPressed);
        w = pb;
        insertOneWidget(w,row,clmn,true);
    }
}

void MainWindow::insertOneWidget(QWidget *w, int &row, int &clmn, bool isKana)
{
    if (w!=nullptr) {
        if (!isKana) {
            ui->gridRad->addWidget(w,row,clmn);
            buttons << w;
        } else {
            ui->gridKana->addWidget(w,row,clmn);
            kanaButtons << w;
        }
        clmn++;
        int max = cgl->maxHButtons;
        if (isKana)
            max = cgl->maxKanaHButtons;
        if (clmn>=max) {
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
        if (pb==nullptr) continue;
        pb->setChecked(false);
        pb->setEnabled(true);
    }
    allowLookup = true;
    radicalPressed(false);
    statusMsg->setText(tr("Ready"));
}

void MainWindow::updateKana(const bool)
{
    renderKanaButtons();
}

void MainWindow::radicalPressed(const bool)
{
    if (!allowLookup) return;

    QList<QPushButton *> pbl;
    pbl.clear();
    for (int i=0;i<buttons.count();i++)
        if (qobject_cast<QPushButton *>(buttons.at(i))!=nullptr)
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

void MainWindow::kanaPressed(const bool)
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn==nullptr) return;

    QString k = btn->text();
    ui->scratchPad->setEditText(ui->scratchPad->currentText()+k);
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
    forceFocusToEdit = false;
    ui->scratchPad->setEditText(text);
}

void MainWindow::screenCapture()
{
#ifdef WITH_OCR
    hide();
    QApplication::processEvents();

    QTimer::singleShot(200, [this](){
        RegionGrabber* rgnGrab = new RegionGrabber(lastGrabbedRegion);
        connect(rgnGrab, &RegionGrabber::regionGrabbed,
                this, &MainWindow::regionGrabbed);
        connect(rgnGrab, &RegionGrabber::regionUpdated,
                this, &MainWindow::regionUpdated);
    });
#endif
}

void MainWindow::regionGrabbed(const QPixmap &pic)
{
#ifdef WITH_OCR
    QString text;

    if ( !pic.isNull() )
    {
        if (ocr!=nullptr && pic.width()>20 && pic.height()>20) {
            QImage cpx = pic.toImage();
            ocr->SetImage(Image2PIX(cpx));
            char* rtext = ocr->GetUTF8Text();
            QString s = QString::fromUtf8(rtext);
            delete[] rtext;
            QStringList sl = s.split('\n',QString::SkipEmptyParts);
            int maxlen = 0;
            for (int i=0;i<sl.count();i++)
                if (sl.at(i).length()>maxlen)
                    maxlen = sl.at(i).length();
            if (maxlen<sl.count()) { // vertical kanji block, needs transpose
                QStringList sl2;
                sl2.clear();
                for (int i=0;i<maxlen;i++)
                    sl2 << QString();
                for (int i=0;i<sl.count();i++)
                    for (int j=0;j<sl.at(i).length();j++)
                        sl2[maxlen-j-1][i]=sl[i][j];
                sl = sl2;
            }
            if (!sl.isEmpty()) {
                text = sl.join(QString(" "));
                text.replace(QRegExp("[\r\n]+")," ");
            }
        }
    }

    RegionGrabber* rgnGrab = qobject_cast<RegionGrabber *>(sender());
    if(rgnGrab)
        rgnGrab->deleteLater();

    QApplication::restoreOverrideCursor();
    restoreWindow();

    if (!text.isEmpty())
        ui->scratchPad->setEditText(text);
#else
    Q_UNUSED(pic)
#endif
}

void MainWindow::regionUpdated(const QRect &region)
{
    lastGrabbedRegion = region;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    cgl->writeSettings(this);
    event->accept();
}

void MainWindow::settingsDlg()
{
    QSettingsDlg *dlg = new QSettingsDlg(this,cgl->fontBtn,cgl->fontLabels,cgl->fontResults,cgl->maxHButtons,
                                         cgl->maxKanaHButtons, cgl->getDictPaths());
    if (dlg->exec()) {
        cgl->fontBtn = dlg->fontBtn;
        cgl->fontLabels = dlg->fontLabels;
        cgl->fontResults = dlg->fontResults;
        cgl->maxHButtons = dlg->maxHButtons;
        cgl->maxKanaHButtons = dlg->maxKanaHButtons;
        allowLookup = false;
        renderRadicalsButtons();
        renderKanaButtons();
        allowLookup = true;
        resetRadicals();
        ui->scratchPad->setFont(cgl->fontResults);
        ui->dictWords->setFont(cgl->fontBtn);
        cgl->setDictPaths(dlg->getDictPaths());
        cgl->wordFinder->clear();
        cgl->loadDictionaries();
    }
    dlg->setParent(nullptr);
    delete dlg;
}

void MainWindow::opacityList()
{
    QAction* ac = qobject_cast<QAction* >(sender());
    if (ac!=nullptr) {
        int op = ac->data().toInt();
        if (op>0 && op<=100)
            setWindowOpacity((qreal)op/100.0);
        return;
    }

    QMenu* m = new QMenu();
    for (int i=0;i<=10;i++) {
        QAction* ac = new QAction(QString("%1%").arg(50+i*5),nullptr);
        connect(ac,&QAction::triggered,this,&MainWindow::opacityList);
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

void MainWindow::wordListLookupItem(QListWidgetItem *item)
{
    QString newValue = item->text();

    ui->scratchPad->setEditText(newValue);
}

void MainWindow::wordListSelectionChanged()
{
    QList< QListWidgetItem * > selected = ui->dictWords->selectedItems();

    if ( selected.size() )
        wordListItemActivated( selected.front() );
}

void MainWindow::dictLoadFinished()
{
    dictView->unsetCursor();

    if (forceFocusToEdit)
        ui->scratchPad->setFocus();
}

void MainWindow::dictLoadUrl(const QUrl &url)
{
    dictView->setSource(url);
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
    dictLoadUrl(req);

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
    dictLoadUrl(req);

    dictView->setCursor( Qt::WaitCursor );
}

CAuxDictKeyFilter::CAuxDictKeyFilter(QObject *parent)
    : QObject(parent)
{

}

bool CAuxDictKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if (ev!=nullptr)
            emit keyPressed(ev->key());
    }
    return QObject::eventFilter(obj,event);
}

void MainWindow::restoreWindow()
{
    showNormal();
    raise();
    activateWindow();
}
