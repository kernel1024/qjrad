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
    explicit QSettingsDlg(QWidget *parent = nullptr);
    ~QSettingsDlg() override;

    QStringList getDictPaths() const;
    void setDictPaths(const QStringList &paths);
    
    QFont getFontBtn() const;
    void setFontBtn(const QFont &value);

    QFont getFontLabels() const;
    void setFontLabels(const QFont &value);

    QFont getFontResults() const;
    void setFontResults(const QFont &value);

    int getMaxHButtons() const;
    void setMaxHButtons(int value);

    int getMaxKanaHButtons() const;
    void setMaxKanaHButtons(int value);

    int getMaxDictionaryResults() const;
    void setMaxDictionaryResults(int value);

private:
    Ui::QSettingsDlg *ui;
    void updateFonts();

public Q_SLOTS:
    void changeFont();
    void addDir();
    void delDir();
};

#endif // SETTINGSDLG_H
