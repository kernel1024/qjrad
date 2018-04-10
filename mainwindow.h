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
#include <QTextBrowser>
#include <QPixmap>

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
    QTextBrowser* dictView;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void renderRadicalsButtons();
    void renderKanaButtons();
    void clearRadButtons();
    void clearKanaButtons();
    QList<int> getSplittersSize();
    QList<int> getDictSplittersSize();
    int getKanjiGrade(const QChar &kanji);

protected:
    QKDictionary dict;

private:
    Ui::MainWindow *ui;
    QObjectList buttons, kanaButtons;
    bool allowLookup;
    QLabel *statusMsg;
    QString infoKanjiTemplate;
    bool forceFocusToEdit;
    CAuxDictKeyFilter *keyFilter;
    QRect lastGrabbedRegion;

    QString lastWordFinderReq;
    bool fuzzySearch;

    void insertOneWidget(QWidget *w, int &row, int &clmn, bool isKana);
    void closeEvent(QCloseEvent * event);

    void showTranslationFor( QString const & inWord );
    void showEmptyTranslationPage();
    void updateMatchResults( bool finished );

    void restoreWindow();
    void startWordSearch(const QString &newValue, bool fuzzy);
public slots:
    // window geometry and misc
    void centerWindow();
    void updateSplitters();
    // event handlers
    void resetRadicals();
    void updateKana(const bool checked);
    void radicalPressed(const bool checked);
    void kanaPressed(const bool checked);
    void settingsDlg();
    void opacityList();
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);
    void setScratchPadText(const QString & text);
    void screenCapture();
    void regionGrabbed(const QPixmap &pic);
    void regionUpdated(const QRect &region);

    // for GoldenDict
    void prefixMatchUpdated();
    void prefixMatchFinished();
    void translateInputChanged( QString const & );
    void translateInputFinished();
    void wordListItemActivated( QListWidgetItem * );
    void wordListLookupItem( QListWidgetItem * item);
    void wordListSelectionChanged();
    void dictLoadFinished();
    void dictLoadUrl(const QUrl& url);

};


#endif // MAINWINDOW_H
