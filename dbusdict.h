#ifndef DBUSDICT_H
#define DBUSDICT_H

#include <QObject>
#include "zdict/zdictcontroller.h"

class ZMainWindow;

class ZKanjiDBusDict : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.qjrad.dictionary")
private:
    ZDict::ZDictController* m_dictManager;
    ZMainWindow* m_wnd;

public:
    explicit ZKanjiDBusDict(QObject *parent, ZDict::ZDictController* dictManager);
    void setMainWindow(ZMainWindow* wnd);

Q_SIGNALS:
    Q_SCRIPTABLE void gotWordTranslation(const QString& html);

public Q_SLOTS:
    void findWordTranslation(const QString& text);
    void showDictionaryWindow(const QString& text);

};

#endif // DBUSDICT_H
