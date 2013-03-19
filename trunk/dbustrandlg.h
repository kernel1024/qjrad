#ifndef DBUSTRANDLG_H
#define DBUSTRANDLG_H

#include <QDialog>
#include <QWebView>

namespace Ui {
class QKDBusTranDlg;
}

class QKDBusTranDlg : public QDialog
{
    Q_OBJECT
    
public:
    QWebView *webView;
    explicit QKDBusTranDlg(QWidget *parent = 0);
    ~QKDBusTranDlg();
    
private:
    Ui::QKDBusTranDlg *ui;
};

#endif // DBUSTRANDLG_H
