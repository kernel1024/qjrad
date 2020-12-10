#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <QDialog>

namespace Ui {
    class QSettingsDlg;
}

class ZSettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ZSettingsDialog(QWidget *parent = nullptr);
    ~ZSettingsDialog() override;
    void loadSettings();
    void saveSettings();

private:
    Ui::QSettingsDlg *ui;
    void updateFonts();
    void updateOCRLanguages();

private Q_SLOTS:
    void changeFont();
    void addDir();
    void delDir();
    void ocrDatapath();
    void rebuildKanjiDict();

Q_SIGNALS:
    void cleanupDictionaries();

};

#endif // SETTINGSDLG_H
