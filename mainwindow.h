#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QString>
#include <QList>
#include <QLabel>
#include <QModelIndex>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QWebEngineView>
#include <QWebEngineProfile>

#include "kdictionary.h"

namespace Ui {
    class MainWindow;
}

class CAuxDictKeyFilter : public QObject
{
    Q_OBJECT
public:
    CAuxDictKeyFilter(QObject *parent = 0);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
signals:
    void keyPressed(int key);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString foundKanji;
    QWebEngineView* dictView;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void renderButtons();
    void clearRadButtons();
    QList<int> getSplittersSize();
    QList<int> getDictSplittersSize();
    int getKanjiGrade(const QChar &kanji);

protected:
    QKDictionary dict;

private:
    Ui::MainWindow *ui;
    QObjectList buttons;
    bool allowLookup;
    QLabel *statusMsg;
    QString infoKanjiTemplate;
    QWebEngineProfile* webProfile;
    bool forceFocusToEdit;
    CAuxDictKeyFilter *keyFilter;

    void insertOneWidget(QWidget *w, int &row, int &clmn);
    void closeEvent(QCloseEvent * event);

    void showTranslationFor( QString const & inWord );
    void showEmptyTranslationPage();
    void updateMatchResults( bool finished );

public slots:
    // window geometry and misc
    void centerWindow();
    void updateSplitters();
    // event handlers
    void resetRadicals();
    void radicalPressed(const bool checked);
    void settingsDlg();
    void opacityList();
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);
    void setScratchPadText(const QString & text);

    // for GoldenDict
    void prefixMatchUpdated();
    void prefixMatchFinished();
    void translateInputChanged( QString const & );
    void translateInputFinished();
    void wordListItemActivated( QListWidgetItem * );
    void wordListSelectionChanged();
    void dictLoadFinished(bool ok);

};


#endif // MAINWINDOW_H
