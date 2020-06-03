#include "settingsdlg.h"
#include "ui_settingsdlg.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QListWidgetItem>

QSettingsDlg::QSettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QSettingsDlg)
{
    ui->setupUi(this);
    connect(ui->btnFontButtons,&QPushButton::clicked,this,&QSettingsDlg::changeFont);
    connect(ui->btnFontLabels,&QPushButton::clicked,this,&QSettingsDlg::changeFont);
    connect(ui->btnFontResults,&QPushButton::clicked,this,&QSettingsDlg::changeFont);
    connect(ui->btnAddPath,&QPushButton::clicked,this,&QSettingsDlg::addDir);
    connect(ui->btnDelPath,&QPushButton::clicked,this,&QSettingsDlg::delDir);

    updateFonts();
}

QSettingsDlg::~QSettingsDlg()
{
    delete ui;
}

QStringList QSettingsDlg::getDictPaths() const
{
    QStringList sl;
    sl.reserve(ui->dictPaths->count());
    for (int i=0;i<ui->dictPaths->count();i++)
        sl << ui->dictPaths->item(i)->text();
    return sl;
}

void QSettingsDlg::setDictPaths(const QStringList &paths)
{
    ui->dictPaths->addItems(paths);
}

void QSettingsDlg::updateFonts()
{
    ui->btnFontButtons->setText(ui->testBtn->font().family());
    ui->btnFontLabels->setText(ui->testLabels->font().family());
    ui->btnFontResults->setText(ui->testResults->font().family());
}

void QSettingsDlg::changeFont()
{
    auto *pb = qobject_cast<QPushButton *>(sender());
    if (pb==nullptr) return;
    bool ok = false;
    QFont f = QApplication::font();
    if (pb==ui->btnFontButtons) {
        f = ui->testBtn->font();
    } else if (pb==ui->btnFontLabels) {
        f = ui->testLabels->font();
    } else if (pb==ui->btnFontResults) {
        f = ui->testResults->font();
    }
    f = QFontDialog::getFont(&ok,f,this);
    if (!ok) return;
    if (pb==ui->btnFontButtons) {
        ui->testBtn->setFont(f);
    } else if (pb==ui->btnFontLabels) {
        ui->testLabels->setFont(f);
    } else if (pb==ui->btnFontResults) {
        ui->testResults->setFont(f);
    }
    updateFonts();
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

QFont QSettingsDlg::getFontBtn() const
{
    return ui->testBtn->font();
}

void QSettingsDlg::setFontBtn(const QFont &value)
{
    ui->testBtn->setFont(value);
    updateFonts();
}

QFont QSettingsDlg::getFontLabels() const
{
    return ui->testLabels->font();
}

void QSettingsDlg::setFontLabels(const QFont &value)
{
    ui->testLabels->setFont(value);
    updateFonts();
}

QFont QSettingsDlg::getFontResults() const
{
    return ui->testResults->font();
}

void QSettingsDlg::setFontResults(const QFont &value)
{
    ui->testResults->setFont(value);
    updateFonts();
}

int QSettingsDlg::getMaxHButtons() const
{
    return ui->buttonsCnt->value();
}

void QSettingsDlg::setMaxHButtons(int value)
{
    ui->buttonsCnt->setValue(value);
}

int QSettingsDlg::getMaxKanaHButtons() const
{
    return ui->buttonsCntKana->value();
}

void QSettingsDlg::setMaxKanaHButtons(int value)
{
    ui->buttonsCntKana->setValue(value);
}

int QSettingsDlg::getMaxDictionaryResults() const
{
    return ui->resultMax->value();
}

void QSettingsDlg::setMaxDictionaryResults(int value)
{
    ui->resultMax->setValue(value);
}
