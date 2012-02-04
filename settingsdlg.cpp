#include <QtGui>
#include "settingsdlg.h"
#include "ui_settingsdlg.h"


QSettingsDlg::QSettingsDlg(QWidget *parent, const QFont &fBtn, const QFont &fLabels, const QFont &fResults) :
    QDialog(parent),
    ui(new Ui::QSettingsDlg)
{
    ui->setupUi(this);
    fontBtn = fBtn;
    fontLabels = fLabels;
    fontResults = fResults;
    connect(ui->btnFontButtons,SIGNAL(clicked()),this,SLOT(changeFont()));
    connect(ui->btnFontLabels,SIGNAL(clicked()),this,SLOT(changeFont()));
    connect(ui->btnFontResults,SIGNAL(clicked()),this,SLOT(changeFont()));
    updateFonts();
}

QSettingsDlg::~QSettingsDlg()
{
    delete ui;
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
