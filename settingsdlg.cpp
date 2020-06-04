#include <QFileDialog>
#include <QFontDialog>
#include <QListWidgetItem>
#include <QSettings>
#include <QMessageBox>
#include "settingsdlg.h"
#include "qsl.h"
#include "global.h"
#include "ui_settingsdlg.h"

ZSettingsDialog::ZSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QSettingsDlg)
{
    ui->setupUi(this);
    connect(ui->btnFontButtons,&QPushButton::clicked,this,&ZSettingsDialog::changeFont);
    connect(ui->btnFontLabels,&QPushButton::clicked,this,&ZSettingsDialog::changeFont);
    connect(ui->btnFontResults,&QPushButton::clicked,this,&ZSettingsDialog::changeFont);
    connect(ui->btnAddPath,&QPushButton::clicked,this,&ZSettingsDialog::addDir);
    connect(ui->btnDelPath,&QPushButton::clicked,this,&ZSettingsDialog::delDir);
    connect(ui->btnOCRDatapath,&QPushButton::clicked,this,&ZSettingsDialog::ocrDatapath);

    updateFonts();

#ifdef WITH_OCR
    updateOCRLanguages();
#else
    ui->groupTesseract->setEnabled(false);
    ui->editOCRDatapath->setEnabled(false);
    ui->comboOCRLanguage->setEnabled(false);
#endif
}

ZSettingsDialog::~ZSettingsDialog()
{
    delete ui;
}

void ZSettingsDialog::updateFonts()
{
    ui->btnFontButtons->setText(ui->testBtn->font().family());
    ui->btnFontLabels->setText(ui->testLabels->font().family());
    ui->btnFontResults->setText(ui->testResults->font().family());
}

void ZSettingsDialog::loadSettings()
{
    ui->dictPaths->clear();
    ui->dictPaths->addItems(zF->getDictPaths());

    ui->testBtn->setFont(zF->fontBtn());
    ui->testLabels->setFont(zF->fontLabels());
    ui->testResults->setFont(zF->fontResults());
    updateFonts();

    QSettings stg;
    stg.beginGroup(QSL("Main"));
    ui->buttonsCnt->setValue(stg.value(QSL("maxHButtons"),CDefaults::maxHButtons).toInt());
    ui->buttonsCntKana->setValue(stg.value(QSL("maxKanaHButtons"),CDefaults::maxKanaHButtons).toInt());
    ui->resultMax->setValue(stg.value(QSL("maxDictionaryResults"),CDefaults::maxDictionaryResults).toInt());
    stg.endGroup();

#ifdef WITH_OCR
    ui->editOCRDatapath->setText(zF->ocrGetDatapath());
    updateOCRLanguages();
#endif
}

void ZSettingsDialog::saveSettings()
{
    QSettings stg;
    stg.beginGroup(QSL("Main"));
    stg.setValue(QSL("fontButton"),ui->testBtn->font());
    stg.setValue(QSL("fontLabel"),ui->testLabels->font());
    stg.setValue(QSL("fontResult"),ui->testResults->font());

    stg.setValue(QSL("maxHButtons"),ui->buttonsCnt->value());
    stg.setValue(QSL("maxKanaHButtons"),ui->buttonsCntKana->value());
    stg.setValue(QSL("maxDictionaryResults"),ui->resultMax->value());
    stg.endGroup();

    stg.beginGroup(QSL("Dictionaries"));
    for (int i=0; i<ui->dictPaths->count(); i++) {
        QString path = ui->dictPaths->item(i)->text();
        if (!path.isEmpty())
            stg.setValue(QSL("%1").arg(i),path);
    }
    stg.endGroup();

#ifdef WITH_OCR
    bool needRestart = (ui->editOCRDatapath->text() != zF->ocrGetDatapath());
    stg.beginGroup(QSL("OCR"));
    stg.setValue(QSL("activeLanguage"), ui->comboOCRLanguage->currentData().toString());
    stg.setValue(QSL("datapath"), ui->editOCRDatapath->text());
    stg.endGroup();

    if (needRestart) {
        QMessageBox::information(this,QGuiApplication::applicationDisplayName(),
                                 tr("OCR datapath changed.\n"
                                    "The application needs to be restarted to apply these settings."));
    }
#endif
}

void ZSettingsDialog::updateOCRLanguages()
{
#ifdef WITH_OCR
    ui->comboOCRLanguage->clear();
    QString selectedLang = zF->ocrGetActiveLanguage();

    QDir datapath(ui->editOCRDatapath->text());
    if (datapath.isReadable()) {
        int aidx = -1;
        const QFileInfoList files = datapath.entryInfoList( { QSL("*.traineddata") } );
        for (const QFileInfo& fi : files) {
            QString base = fi.baseName();
            ui->comboOCRLanguage->addItem(fi.fileName(),base);
            if (base==selectedLang)
                aidx = ui->comboOCRLanguage->count()-1;
        }

        if (aidx>=0) {
            ui->comboOCRLanguage->setCurrentIndex(aidx);
        } else if (ui->comboOCRLanguage->count()>0) {
            ui->comboOCRLanguage->setCurrentIndex(0);
        }
    }
#endif
}

void ZSettingsDialog::ocrDatapath()
{
#ifdef WITH_OCR
    QString datapath = QFileDialog::getExistingDirectory(this,tr("Tesseract datapath"),ui->editOCRDatapath->text());
    if (!datapath.isEmpty())
        ui->editOCRDatapath->setText(datapath);
    updateOCRLanguages();
#endif
}

void ZSettingsDialog::changeFont()
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

void ZSettingsDialog::addDir()
{
    QString s = QFileDialog::getExistingDirectory(this,tr("Select directory"));
    if (!s.isEmpty())
        ui->dictPaths->addItem(s);
}

void ZSettingsDialog::delDir()
{
    int idx = ui->dictPaths->currentRow();
    if (idx<0 || idx>=ui->dictPaths->count()) return;
    QListWidgetItem *a = ui->dictPaths->takeItem(idx);
    delete a;
}
