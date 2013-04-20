#ifndef DBUSDICT_H
#define DBUSDICT_H

#include <QObject>
#include "goldendictmgr.h"

class QKDBusDict : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.qjrad.dictionary")
private:
    ArticleNetworkAccessManager* netMan;

public:
    explicit QKDBusDict(QObject *parent, ArticleNetworkAccessManager* netManager);

signals:
    Q_SCRIPTABLE void gotWordTranslation(const QString& html);

protected slots:
    void dataReady();

public slots:
    void findWordTranslation(const QString& text);

};

#endif // DBUSDICT_H