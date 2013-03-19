#ifndef DBUSDICT_H
#define DBUSDICT_H

#include <QObject>
#include "dbustrandlg.h"

class QKDBusDict : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.qjrad.dictionary")
private:
    QKDBusTranDlg* hdlg;
public:
    explicit QKDBusDict(QObject *parent = 0);
    ~QKDBusDict();

signals:
    Q_SCRIPTABLE void gotWordTranslation(const QString& html);

protected slots:
    void loadFinished(bool ok);

public slots:
    void findWordTranslation(const QString& text);

};

#endif // DBUSDICT_H
