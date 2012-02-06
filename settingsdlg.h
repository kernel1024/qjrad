#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <QDialog>

namespace Ui {
    class QSettingsDlg;
}

class QSettingsDlg : public QDialog
{
    Q_OBJECT
    
public:
    QFont fontBtn, fontLabels, fontResults;
    int maxHButtons;
    explicit QSettingsDlg(QWidget *parent, const QFont &fBtn, const QFont &fLabels, const QFont &fResults,
                          int aMaxHButtons);
    ~QSettingsDlg();
    
private:
    Ui::QSettingsDlg *ui;
    void updateFonts();

public slots:
    void changeFont();
    void cntChanged(int i);
};

#endif // SETTINGSDLG_H
