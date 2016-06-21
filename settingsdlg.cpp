#include "settingsdlg.h"
#include "ui_settingsdlg.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QListWidgetItem>

QSettingsDlg::QSettingsDlg(QWidget *parent, const QFont &fBtn, const QFont &fLabels, const QFont &fResults,
                           int aMaxHButtons, int aMaxKanaHButtons, const QStringList &dictPaths) :
    QDialog(parent),
    ui(new Ui::QSettingsDlg)
{
    ui->setupUi(this);
    fontBtn = fBtn;
    fontLabels = fLabels;
    fontResults = fResults;
    maxHButtons = aMaxHButtons;
    maxKanaHButtons = aMaxKanaHButtons;
    ui->buttonsCnt->setValue(maxHButtons);
    ui->buttonsCntKana->setValue(maxKanaHButtons);
    connect(ui->btnFontButtons,SIGNAL(clicked()),this,SLOT(changeFont()));
    connect(ui->btnFontLabels,SIGNAL(clicked()),this,SLOT(changeFont()));
    connect(ui->btnFontResults,SIGNAL(clicked()),this,SLOT(changeFont()));
    connect(ui->buttonsCnt,SIGNAL(valueChanged(int)),this,SLOT(cntChanged(int)));
    connect(ui->buttonsCntKana,SIGNAL(valueChanged(int)),this,SLOT(cntChangedKana(int)));
    connect(ui->btnAddPath,SIGNAL(clicked()),this,SLOT(addDir()));
    connect(ui->btnDelPath,SIGNAL(clicked()),this,SLOT(delDir()));

    ui->dictPaths->addItems(dictPaths);
    updateFonts();
}

QSettingsDlg::~QSettingsDlg()
{
    delete ui;
}

QStringList QSettingsDlg::getDictPaths()
{
    QStringList sl;
    for (int i=0;i<ui->dictPaths->count();i++)
        sl << ui->dictPaths->item(i)->text();
    return sl;
}

void QSettingsDlg::updateFonts()
{
    ui->testBtn->setFont(fontBtn);
    ui->testLabels->setFont(fontLabels);
    ui->testResults->setFont(fontResults);
    ui->btnFontButtons->setText(fontBtn.family());
    ui->btnFontLabels->setText(fontLabels.family());
    ui->btnFontResults->setText(fontResults.family());
}

void QSettingsDlg::changeFont()
{
    QPushButton *pb = qobject_cast<QPushButton *>(sender());
    if (pb==NULL) return;
    bool ok;
    QFont f = QApplication::font();
    if (pb==ui->btnFontButtons)
        f = fontBtn;
    else if (pb==ui->btnFontLabels)
        f = fontLabels;
    else if (pb==ui->btnFontResults)
        f = fontResults;
    f = QFontDialog::getFont(&ok,f,this);
    if (!ok) return;
    if (pb==ui->btnFontButtons)
        fontBtn = f;
    else if (pb==ui->btnFontLabels)
        fontLabels = f;
    else if (pb==ui->btnFontResults)
        fontResults = f;
    updateFonts();
}

void QSettingsDlg::cntChanged(int i)
{
    maxHButtons = i;
}

void QSettingsDlg::cntChangedKana(int i)
{
    maxKanaHButtons = i;
}

void QSettingsDlg::addDir()
{
    QString s = QFileDialog::getExistingDirectory(this,tr("Select directory"));
    if (!s.isEmpty())
        ui->dictPaths->addItem(s);
}

void QSettingsDlg::delDir()
{
    int idx = ui->dictPaths->currentRow();
    if (idx<0 || idx>=ui->dictPaths->count()) return;
    QListWidgetItem *a = ui->dictPaths->takeItem(idx);
    delete a;
}
