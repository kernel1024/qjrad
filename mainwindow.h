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
Q_SIGNALS:
    void keyPressed(int key);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString foundKanji;
    QTextBrowser* dictView { nullptr }; // TODO: remove?

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() override;

    void renderRadicalsButtons();
    void renderKanaButtons();
    void clearRadButtons();
    void clearKanaButtons();
    QList<int> getSplittersSize();
    QList<int> getDictSplittersSize();
    int getKanjiGrade(QChar kanji) const;

private:
    Ui::MainWindow *ui;
    QObjectList buttons, kanaButtons;
    bool allowLookup;
    QLabel *statusMsg;
    QString infoKanjiTemplate;
    bool forceFocusToEdit;
    CAuxDictKeyFilter *keyFilter;
    QRect lastGrabbedRegion;
    QKDictionary dict;

    QString lastWordFinderReq;
    bool fuzzySearch;

    void insertOneWidget(QWidget *w, int &row, int &clmn, bool isKana);
    void closeEvent(QCloseEvent * event) override;

    void showTranslationFor(const QString &word) const;
    void restoreWindow();
    void startWordSearch(const QString &newValue, bool fuzzy);
    void updateResultsCountLabel();

public Q_SLOTS:
    // window geometry and misc
    void centerWindow();
    void updateSplitters();
    // event handlers
    void resetRadicals();
    void updateKana(bool checked);
    void radicalPressed(bool checked);
    void kanaPressed(bool checked);
    void settingsDlg();
    void opacityList();
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);
    void setScratchPadText(const QString & text);
    void screenCapture();
    void regionGrabbed(const QPixmap &pic);
    void regionUpdated(const QRect &region);

    void updateMatchResults(const QStringList &words);
    void translateInputChanged(const QString &newValues);
    void translateInputFinished();
    void wordListLookupItem(QListWidgetItem * item);
    void wordListSelectionChanged();
    void dictLoadFinished();
    void articleReady(const QString& text) const;
    void articleLinkClicked(const QUrl& url);

Q_SIGNALS:
    void stopDictionaryWork();

};


#endif // MAINWINDOW_H
