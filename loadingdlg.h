#ifndef LOADINGDLG_H
#define LOADINGDLG_H

#include <QDialog>

namespace Ui {
class CLoading;
}

class CLoading : public QDialog
{
    Q_OBJECT
    
public:
    explicit CLoading(QWidget *parent = 0);
    ~CLoading();
    
private:
    Ui::CLoading *ui;

public slots:
    void addLogMsg(const QString& msg);

};

#endif // LOADINGDLG_H
