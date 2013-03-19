#include "dbustrandlg.h"
#include "ui_dbustrandlg.h"

QKDBusTranDlg::QKDBusTranDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QKDBusTranDlg)
{
    ui->setupUi(this);
    webView = ui->webView;
}

QKDBusTranDlg::~QKDBusTranDlg()
{
    webView = NULL;
    delete ui;
}
