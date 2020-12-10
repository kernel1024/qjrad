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

class ZMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString foundKanji;

    explicit ZMainWindow(QWidget *parent = 0);
    ~ZMainWindow() override;

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
    ZKanjiDictionary dict;

    QString lastWordFinderReq;
    bool fuzzySearch;

    void insertOneWidget(QWidget *w, int &row, int &clmn, bool isKana);

    void showTranslationFor(const QString &word) const;
    void restoreWindow();
    void startWordSearch(const QString &newValue, bool fuzzy);
    void updateResultsCountLabel();

public Q_SLOTS:
    // Window geometry
    void centerWindow();
    void updateSplitters();

    // GUI handlers
    void settingsDlg();
    void setupDictionaries();

    // Kanji dictionary / table
    void resetRadicals();
    void updateKana(bool checked);
    void radicalPressed(bool checked);
    void kanaPressed(bool checked);
    void opacityList();
    void kanjiClicked(const QModelIndex & index);
    void kanjiAdd(const QModelIndex & index);
    void setScratchPadText(const QString & text);

    // OCR
    void screenCapture();
    void regionGrabbed(const QPixmap &pic);
    void regionUpdated(const QRect &region);

    // Word dictionary
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
