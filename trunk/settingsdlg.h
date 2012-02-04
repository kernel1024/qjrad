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
    explicit QSettingsDlg(QWidget *parent, const QFont &fBtn, const QFont &fLabels, const QFont &fResults);
    ~QSettingsDlg();
    
private:
    Ui::QSettingsDlg *ui;
    void updateFonts();

public slots:
    void changeFont();
};

#endif // SETTINGSDLG_H
