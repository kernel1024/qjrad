#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include "kdictionary.h"
#include "goldendictmgr.h"
#include "goldendict/wordfinder.hh"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString foundKanji;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void renderButtons();
    void clearRadButtons();
    QList<int> getSplittersSize();
    QList<int> getDictSplittersSize();
    int getKanjiGrade(const QChar &kanji);

protected:
    QKDictionary dict;
    ArticleNetworkAccessManager * netMan;
    CGoldenDictMgr * dictManager;
    WordFinder * wordFinder;


private:
    Ui::MainWindow *ui;
    QObjectList buttons;
    bool allowLookup;
    QLabel *statusMsg;
    QString infoKanjiTemplate;

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
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);

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
