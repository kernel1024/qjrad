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
    int maxHButtons, maxKanaHButtons, maxDictionaryResults;
    explicit QSettingsDlg(QWidget *parent, const QFont &fBtn, const QFont &fLabels, const QFont &fResults,
                          int aMaxHButtons, int aMaxKanaHButtons, int aMaxDictionaryResults,
                          const QStringList& dictPaths);
    ~QSettingsDlg();
    QStringList getDictPaths();
    
private:
    Ui::QSettingsDlg *ui;
    void updateFonts();

public slots:
    void changeFont();
    void cntChanged(int i);
    void cntChangedKana(int i);
    void cntChangedMaxResults(int i);
    void addDir();
    void delDir();
};

#endif // SETTINGSDLG_H
