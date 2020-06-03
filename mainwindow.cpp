#include <QMessageBox>
#include <QLineEdit>
#include <QDesktopWidget>
#include <QUrl>
#include <QScrollBar>
#include <QMenu>
#include <QUrlQuery>
#include <QTimer>
#include <QWindow>
#include <QScreen>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kanjimodel.h"
#include "settingsdlg.h"
#include "miscutils.h"
#include "global.h"
#include "dbusdict.h"
#include "regiongrabber.h"
#include "zdict/zdictcontroller.h"

namespace CDefaults {
const int btnWidthMultiplier = 13;
const int btnWidthDivider = 10;
const int screenCaptureDelay = 200;
const int splittersEnforcingDelay = 1000;
const int statusBarMessageMinWidth = 150;
const int radicalsColorBiasMultiplier = 25;
}

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

    infoKanjiTemplate = ui->infoKanji->toHtml();
    ui->infoKanji->clear();

    if (!dict.loadDictionaries()) {
        QMessageBox::critical(this,tr("QJRad - error"),
                              tr("Cannot load main dictionaries\nError: %1").arg(dict.errorString));
    }

    QIcon appicon;
    const QVector<int> iconSizes({22,32,48,64,128});
    for (const auto sz : iconSizes)
        appicon.addFile(QSL(":/data/appicon%1.png").arg(sz),QSize(sz,sz));
    setWindowIcon(appicon);

    foundKanji.clear();

    statusMsg = new QLabel(tr("Ready"));
    statusMsg->setMinimumWidth(CDefaults::statusBarMessageMinWidth);
    statusMsg->setAlignment(Qt::AlignCenter);
    statusBar()->addPermanentWidget(statusMsg);

    connect(ui->btnReset,&QPushButton::clicked,this,&MainWindow::resetRadicals);
    connect(ui->btnSettings,&QPushButton::clicked,this,&MainWindow::settingsDlg);
    connect(ui->btnOpacity,&QPushButton::clicked,this,&MainWindow::opacityList);
    connect(ui->listKanji,&QListView::clicked,this,&MainWindow::kanjiClicked);
    connect(ui->listKanji,&QListView::doubleClicked,this,&MainWindow::kanjiAdd);
    connect(ui->clearScratch,&QPushButton::clicked,ui->scratchPad,&QComboBox::clearEditText);
    connect(ui->btnCapture,&QPushButton::clicked,this,&MainWindow::screenCapture);

    connect(ui->btnBackspace,&QPushButton::clicked,[this](){
        if (!ui->scratchPad->currentText().isEmpty()) {
            ui->scratchPad->setEditText(ui->scratchPad->currentText().left(
                                            ui->scratchPad->currentText().length()-1));
        }
    });

    connect(ui->scratchPad,&QComboBox::editTextChanged,this,&MainWindow::translateInputChanged);
    connect(ui->scratchPad->lineEdit(),&QLineEdit::returnPressed,this,&MainWindow::translateInputFinished);
    connect(ui->dictWords,&QListWidget::itemSelectionChanged,this,&MainWindow::wordListSelectionChanged);
    connect(ui->dictWords,&QListWidget::itemDoubleClicked,this,&MainWindow::wordListLookupItem);

    connect(cgl->dictManager,&ZDict::ZDictController::wordListComplete,
            this,&MainWindow::updateMatchResults,Qt::QueuedConnection);
    connect(cgl->dictManager,&ZDict::ZDictController::articleComplete,
            this,&MainWindow::articleReady,Qt::QueuedConnection);
    connect(this,&MainWindow::stopDictionaryWork,cgl->dictManager,&ZDict::ZDictController::cancelActiveWork);

    connect(dictView, &QTextBrowser::textChanged,this,&MainWindow::dictLoadFinished);

    keyFilter = new CAuxDictKeyFilter(this);
    ui->scratchPad->installEventFilter(keyFilter);
    connect(keyFilter, &CAuxDictKeyFilter::keyPressed, [this](int key){
        Q_UNUSED(key)
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

    dictView->clear();
    lastWordFinderReq.clear();
    fuzzySearch = false;
    cgl->loadDictionaries();

    QTimer::singleShot(CDefaults::splittersEnforcingDelay,this,&MainWindow::updateSplitters);
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

    QScreen *screen = nullptr;

    if (window() && window()->windowHandle()) {
        screen = window()->windowHandle()->screen();
    } else if (!QApplication::screens().isEmpty()) {
        screen = QApplication::screenAt(QCursor::pos());
    }
    if (screen == nullptr)
        screen = QApplication::primaryScreen();

    QRect rect(screen->availableGeometry());
    move(rect.width()/2 - frameGeometry().width()/2,
         rect.height()/2 - frameGeometry().height()/2);
}

void MainWindow::updateSplitters()
{
    ui->splitter->setCollapsible(0,true);

    QList<int> widths({ width() - cgl->savedDictSplitterPos,
                        cgl->savedDictSplitterPos });

    ui->splitterMain->setCollapsible(1,true);
    ui->splitterMain->setSizes(widths);

    QList<int> dictWidths({ 2*height()/5,
                            3*height()/5 });
    ui->splitterDict->setSizes(dictWidths);
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

int MainWindow::getKanjiGrade(QChar kanji) const
{
    if (dict.kanjiGrade.contains(kanji))
        return dict.kanjiGrade.value(kanji);

    return 0;
}

void MainWindow::renderRadicalsButtons()
{
    int btnWidth = -1;

    clearRadButtons();

    ui->gridRad->setHorizontalSpacing(2);
    ui->gridRad->setVerticalSpacing(2);
    int rmark=0;
    int row=0;
    int clmn=0;

    QWidget *w = nullptr;
    for (int i=0;i<dict.radicalsLookup.count();i++) {
        QKRadItem ri = dict.radicalsLookup.at(i);
        w = nullptr;
        if (rmark!=ri.strokes) {
            // insert label
            auto *rl = new QLabel(tr("%1").arg(ri.strokes),ui->frameRad);
            rl->setAlignment(Qt::AlignCenter);
            rl->setFont(cgl->fontLabels);
            rl->setFrameShape(QFrame::Box);
            w = rl;
            rmark=ri.strokes;
            insertOneWidget(w,row,clmn,false);
        }
        // insert button
        auto *pb = new QPushButton(ri.radical,ui->frameRad);
        pb->setFlat(true);
        pb->setFont(cgl->fontBtn);
        pb->setCheckable(true);
        // qt 4.8 bug with kde color configuration tool. disabled color is still incorrect, use our specific color
        QPalette p = pb->palette();
        p.setBrush(QPalette::Disabled,QPalette::ButtonText,QBrush(
                       middleColor(QApplication::palette("QPushButton").color(QPalette::Button),
                                   QApplication::palette("QPushButton").color(QPalette::ButtonText),
                                   CDefaults::radicalsColorBiasMultiplier)));
        pb->setPalette(p);
        // ----
        if (btnWidth<0) {
            QFontMetrics fm(pb->font());
            btnWidth = CDefaults::btnWidthMultiplier * fm.horizontalAdvance(CDefaults::biggestRadical)
                       / CDefaults::btnWidthDivider;
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
    static const QList<int> kana = {
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
    const int katakanaOffset = 0x30A0-0x3040;

    int btnWidth = -1;

    clearKanaButtons();

    ui->gridKana->setHorizontalSpacing(2);
    ui->gridKana->setVerticalSpacing(2);
    int row=0;
    int clmn=0;

    QWidget *w = nullptr;
    for(const auto &i : kana) {
        // insert button
        QChar k(i);
        if (ui->radioKatakana->isChecked())
            k = QChar(i+katakanaOffset);
        auto *pb = new QPushButton(k,ui->frameKana);
        pb->setFlat(true);
        pb->setFont(cgl->fontBtn);
        if (btnWidth<0) {
            QFontMetrics fm(pb->font());
            btnWidth = CDefaults::btnWidthMultiplier * fm.horizontalAdvance(CDefaults::biggestRadical)
                       / CDefaults::btnWidthDivider;
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
        auto *pb = qobject_cast<QPushButton *>(buttons.at(i));
        if (pb==nullptr) continue;
        pb->setChecked(false);
        pb->setEnabled(true);
    }
    allowLookup = true;
    radicalPressed(false);
    statusMsg->setText(tr("Ready"));
}

void MainWindow::updateKana(bool checked)
{
    Q_UNUSED(checked)
    renderKanaButtons();
}

void MainWindow::radicalPressed(bool checked)
{
    Q_UNUSED(checked)
    if (!allowLookup) {
        if (!lastWordFinderReq.isEmpty())
            startWordSearch(lastWordFinderReq, false);

        return;
    }

    QList<QPushButton *> buttonList;
    buttonList.reserve(buttons.count());
    for (int i=0;i<buttons.count();i++) {
        auto *btn = qobject_cast<QPushButton *>(buttons.at(i));
        if (btn!=nullptr)
            buttonList.append(btn);
    }

    QStringList kanjiList;
    kanjiList.reserve(buttonList.count());

    // get kanji for each selected radical
    int bpcnt = 0;
    for (int i=0;i<buttonList.count();i++) {
        QPushButton *pb = buttonList.at(i);
        QChar r = pb->text().at(0);
        pb->setEnabled(true);
        if (pb->isChecked()) {
            bpcnt++;
            if (!r.isNull()) {
                int idx = dict.radicalsLookup.indexOf(QKRadItem(r));
                if (idx>=0) {
                    kanjiList.append(dict.radicalsLookup.at(idx).kanji);
                }
            }
        }
    }

    // leave only kanji present for all selected radicals
    while (kanjiList.count()>1) {
        QString fs = kanjiList.at(0);
        QString ms = kanjiList.takeLast();
        int i=0;
        while (i<fs.length()) {
            if (!ms.contains(fs.at(i))) {
                fs = fs.remove(i,1);
            } else {
                i++;
            }
        }
        if (fs.isEmpty()) {
            kanjiList.clear();
            kanjiList << QString();
            break;
        }
        kanjiList.replace(0,fs);
    }

    if (!kanjiList.isEmpty()) {
        // sort kanji by radicals weight and by unicode weight
        foundKanji = dict.sortKanji(kanjiList.takeFirst());
        // disable radicals that not appears on found set entirely
        QList<QChar> availableRadicals;
        for (const auto &kj : qAsConst(foundKanji)) {
            if (dict.kanjiParts.contains(kj)) {
                const QString krad = dict.kanjiParts.value(kj).join(QString());
                for (const auto &rad : qAsConst(krad)) {
                    if (!availableRadicals.contains(rad))
                        availableRadicals.append(rad);
                }
            }
        }
        for (int i=0;i<buttonList.count();i++) {
            if (!availableRadicals.contains(buttonList.at(i)->text().at(0)))
                buttonList.at(i)->setEnabled(false);
        }
        // insert unselectable labels between kanji groups with different stroke count
        int idx = 0;
        int prevsc = 0;
        while (idx<foundKanji.length()) {
            if (prevsc!=dict.kanjiStrokes.value(foundKanji.at(idx))) {
                prevsc = dict.kanjiStrokes.value(foundKanji.at(idx));
                foundKanji.insert(idx,QChar(CDefaults::enclosedNumericsStart+prevsc)); // use enclosed numerics set from unicode
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

    if (bpcnt>0) {
        statusMsg->setText(tr("Found %1 kanji").arg(foundKanji.length()));
    } else {
        statusMsg->setText(tr("Ready"));
    }

    ui->listKanji->verticalScrollBar()->setSingleStep(ui->listKanji->verticalScrollBar()->pageStep());

    if (!lastWordFinderReq.isEmpty())
        startWordSearch(lastWordFinderReq, !foundKanji.isEmpty());
}

void MainWindow::kanaPressed(bool checked)
{
    Q_UNUSED(checked)
    auto *btn = qobject_cast<QPushButton *>(sender());
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
    int strokes = dict.kanjiStrokes.value(k);
    int grade = dict.kanjiGrade.value(k);

    QString msg = QString(infoKanjiTemplate)
                  .arg(cgl->fontResults.family())
                  .arg(ki.kanji)
                  .arg(strokes)
                  .arg(ki.parts.join(QSL(" ")))
                  .arg(grade)
                  .arg(ki.onReading.join(QSL(", ")),
                       ki.kunReading.join(QSL(", ")),
                       ki.meaning.join(QSL(", ")));

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

    QTimer::singleShot(CDefaults::screenCaptureDelay,this,[this](){
        auto* rgnGrab = new RegionGrabber(this, lastGrabbedRegion);
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
    const int minOCRPicSize = 20;
    QString text;

    if ( !pic.isNull() )
    {
        if (ocr!=nullptr && pic.width()>minOCRPicSize && pic.height()>minOCRPicSize) {
            QImage cpx = pic.toImage();
            ocr->SetImage(Image2PIX(cpx));
            char* rtext = ocr->GetUTF8Text();
            QString s = QString::fromUtf8(rtext);
            delete[] rtext;
            QStringList sl = s.split('\n',QString::SkipEmptyParts);
            int maxlen = 0;
            for (const auto &i : qAsConst(sl)) {
                if (i.length()>maxlen)
                    maxlen = i.length();
            }
            if (maxlen<sl.count()) { // vertical kanji block, needs transpose
                QStringList sl2;
                sl2.reserve(maxlen);
                for (int i=0;i<maxlen;i++)
                    sl2 << QString();
                for (int i=0;i<sl.count();i++) {
                    for (int j=0;j<sl.at(i).length();j++)
                        sl2[maxlen-j-1][i]=sl[i][j];
                }
                sl = sl2;
            }
            if (!sl.isEmpty()) {
                text = sl.join(QChar(' '));
                text.replace(QRegularExpression(QSL("[\r\n]+")),QSL(" "));
            }
        }
    }

    auto* rgnGrab = qobject_cast<RegionGrabber *>(sender());
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
    QSettingsDlg dlg(this);
    dlg.setFontBtn(cgl->fontBtn);
    dlg.setFontLabels(cgl->fontLabels);
    dlg.setFontResults(cgl->fontResults);
    dlg.setMaxHButtons(cgl->maxHButtons);
    dlg.setMaxKanaHButtons(cgl->maxKanaHButtons);
    dlg.setMaxDictionaryResults(cgl->maxDictionaryResults);
    dlg.setDictPaths(cgl->getDictPaths());
    if (dlg.exec() == QDialog::Accepted) {
        cgl->fontBtn = dlg.getFontBtn();
        cgl->fontLabels = dlg.getFontLabels();
        cgl->fontResults = dlg.getFontResults();
        cgl->maxHButtons = dlg.getMaxHButtons();
        cgl->maxKanaHButtons = dlg.getMaxKanaHButtons();
        cgl->maxDictionaryResults = dlg.getMaxDictionaryResults();
        allowLookup = false;
        renderRadicalsButtons();
        renderKanaButtons();
        allowLookup = true;
        resetRadicals();
        ui->scratchPad->setFont(cgl->fontResults);
        ui->dictWords->setFont(cgl->fontBtn);
        cgl->setDictPaths(dlg.getDictPaths());
        lastWordFinderReq.clear();
        fuzzySearch = false;
        Q_EMIT stopDictionaryWork();
        cgl->loadDictionaries();
    }
}

void MainWindow::opacityList()
{
    const int opacityMin = 50;
    const int opacityMax = 100;

    auto* ac = qobject_cast<QAction* >(sender());
    if (ac!=nullptr) {
        int op = ac->data().toInt();
        if (op>0 && op<=opacityMax)
            setWindowOpacity(static_cast<qreal>(op/100.0));
        return;
    }

    QMenu m;
    for (int i=opacityMin, inc=5; i<=opacityMax; i+=inc) {
        auto* ac = new QAction(QSL("%1%").arg(i),nullptr);
        connect(ac,&QAction::triggered,this,&MainWindow::opacityList);
        ac->setData(i);
        m.addAction(ac);
    }

    m.exec(QCursor::pos());
}

void MainWindow::wordListLookupItem(QListWidgetItem *item)
{
    QString newValue = item->text();
    ui->scratchPad->setEditText(newValue);
    translateInputFinished();
}

void MainWindow::wordListSelectionChanged()
{
    QList<QListWidgetItem *> selected = ui->dictWords->selectedItems();

    if (!selected.isEmpty() )
        showTranslationFor(selected.front()->text());
}

void MainWindow::dictLoadFinished()
{
    if (forceFocusToEdit)
        ui->scratchPad->setFocus();
}

void MainWindow::articleReady(const QString &text) const
{
    if (!text.isEmpty())
        dictView->setHtml(text);
}

void MainWindow::articleLinkClicked(const QUrl &url)
{
    QUrlQuery requ(url);
    QString word = requ.queryItemValue(QSL("word"));
    if (word.startsWith('%')) {
        QByteArray bword = word.toLatin1();
        if (!bword.isNull() && !bword.isEmpty())
            word = QUrl::fromPercentEncoding(bword);
    }

    if (!word.isEmpty())
        showTranslationFor(word);
}

void MainWindow::updateMatchResults(const QStringList& words)
{
    QStringList results;

    QString subKanji = foundKanji;
    subKanji.remove(QRegularExpression(QSL("[\\x2460-\\x24FF]"))); // remove enclosed numeric marks

    if (fuzzySearch && !subKanji.isEmpty()) { // radicals is pressed, new kanji search in progress
        QRegularExpression pattern(QSL("%1[%2]").arg(lastWordFinderReq,subKanji));

        for (const auto &rawWord : words) {
            if (rawWord == lastWordFinderReq ||       // requested word itself or
                    rawWord.indexOf(pattern)==0) {  // started with requested word and selected kanji
                results.append(rawWord);
            }
        }
    } else {
        results = words;
    }

    ui->dictWords->setUpdatesEnabled(false);

    for(int i = 0; i<results.count(); i++ )
    {
        QListWidgetItem *item = ui->dictWords->item(i);

        if (item == nullptr) {
            item = new QListWidgetItem(results.at(i), ui->dictWords);
            ui->dictWords->addItem(item);
        } else {
            if (item->text() != results.at(i))
                item->setText(results.at(i));
        }
    }

    while (ui->dictWords->count() > results.count()) {
        QListWidgetItem * i = ui->dictWords->takeItem(ui->dictWords->count() - 1);
        if (!i)
            break;

        delete i;
    }

    if (ui->dictWords->count() > 0) {
        ui->dictWords->scrollToItem(ui->dictWords->item(0), QAbstractItemView::PositionAtTop);
        ui->dictWords->setCurrentItem(nullptr, QItemSelectionModel::Clear);
    }

    ui->dictWords->setUpdatesEnabled(true);

    updateResultsCountLabel();
}

void MainWindow::translateInputChanged(const QString &newValue)
{
    if ((ui->scratchPad->findText(newValue)<0) && !newValue.isEmpty())
        ui->scratchPad->addItem(ui->scratchPad->currentText());

    startWordSearch(newValue, false);
}

void MainWindow::startWordSearch(const QString &newValue, bool fuzzy)
{
    Q_EMIT stopDictionaryWork();

    if (ui->dictWords->selectionModel()->hasSelection())
        ui->dictWords->setCurrentItem(nullptr, QItemSelectionModel::Clear);

    QString req = newValue.trimmed();
    if (req.isEmpty()) {
        lastWordFinderReq.clear();
        fuzzySearch = false;
        ui->dictWords->clear();
        updateResultsCountLabel();
        return;
    }

    lastWordFinderReq = req;
    fuzzySearch = fuzzy;
    cgl->dictManager->wordLookupAsync(req,false,cgl->maxDictionaryResults);
}

void MainWindow::updateResultsCountLabel()
{
    if (ui->dictWords->count()>0) {
        ui->dictBox->setTitle(tr("Dictionary (%1 results)").arg(ui->dictWords->count()));
    } else {
        ui->dictBox->setTitle(tr("Dictionary"));
    }
}

void MainWindow::translateInputFinished()
{
    QString word = ui->scratchPad->currentText();

    if (!word.isEmpty() )
        showTranslationFor(word);
}

void MainWindow::showTranslationFor(const QString &word) const
{
    dictView->clear();
    cgl->dictManager->loadArticleAsync(word);
}

CAuxDictKeyFilter::CAuxDictKeyFilter(QObject *parent)
    : QObject(parent)
{

}

bool CAuxDictKeyFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type()==QEvent::KeyPress) {
        auto *ev = dynamic_cast<QKeyEvent *>(event);
        if (ev!=nullptr)
            Q_EMIT keyPressed(ev->key());
    }
    return QObject::eventFilter(obj,event);
}

void MainWindow::restoreWindow()
{
    showNormal();
    raise();
    activateWindow();
}
