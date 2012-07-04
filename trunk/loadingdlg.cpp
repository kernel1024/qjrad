#include "loadingdlg.h"
#include "ui_loadingdlg.h"

CLoading::CLoading(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CLoading)
{
    ui->setupUi(this);
    ui->log->clear();
}

CLoading::~CLoading()
{
    delete ui;
}

void CLoading::addLogMsg(const QString &msg)
{
    ui->log->appendPlainText(msg);
}
