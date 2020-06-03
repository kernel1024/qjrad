#ifndef DBUSDICT_H
#define DBUSDICT_H

#include <QObject>
#include "zdict/zdictcontroller.h"

class MainWindow;

class QKDBusDict : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.qjrad.dictionary")
private:
    ZDict::ZDictController* m_dictManager;
    MainWindow* m_wnd;

public:
    explicit QKDBusDict(QObject *parent, ZDict::ZDictController* dictManager);
    void setMainWindow(MainWindow* wnd);

Q_SIGNALS:
    Q_SCRIPTABLE void gotWordTranslation(const QString& html);

public Q_SLOTS:
    void findWordTranslation(const QString& text);
    void showDictionaryWindow(const QString& text);

};

#endif // DBUSDICT_H
