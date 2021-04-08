#ifndef HIGHLIGHTDIALOG_H
#define HIGHLIGHTDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSlider>
#include <QGroupBox>
#include <QLabel>

class MainWindow;
class ClientSettings;
class HighlightGeneralTab;
class HighlightTextTab;
class HighlightAlertTab;

namespace Ui {
    class HighlightDialog;
}

class HighlightDialog : public QDialog {
    Q_OBJECT

public:
    explicit HighlightDialog(QWidget *parent = 0);
    ~HighlightDialog();

    MainWindow* getMainWindow();

    /* general tab items */
    QListWidget* getGeneralList();
    QGroupBox* getGeneralAlertGroup();
    QComboBox* getGeneralFileSelect();
    QPushButton* getGeneralPlayButton();

    /* text tab items */
    QPushButton* getTextAddButton();
    QPushButton* getTextRemoveButton();
    QListWidget* getTextList();
    QComboBox* getTextFileSelect();
    QPushButton* getTextPlayButton();
    QComboBox* getTextActionSelect();
    QGroupBox* getTextTimerGroup();
    QLineEdit* getTextTimerValue();
    QGroupBox* getTextAlertGroup();    
    QCheckBox* getTextEntireRow();
    QCheckBox* getTextPartialMatch();
    QCheckBox* getTextGroupsMatch();
    QCheckBox* getTextCaseInsensitive();
    QCheckBox* getTextStartingWith();
    QComboBox* getTextHighlightGroup();
    QComboBox* getTextHighlightSortBy();
    QLineEdit* getTextHighlightFilter();
    QGroupBox* getTextCommandGroup();
    QLineEdit* getTextCommandValue();

    /* alert tab items */
    QGroupBox* getBleedingGroup();
    QPushButton* getBleedingPlayButton();
    QComboBox* getBleedingPlaySelect();
    QGroupBox* getStunGroup();
    QPushButton* getStunPlayButton();
    QComboBox* getStunSelect();
    QGroupBox* getHealthGroup();
    QPushButton* getHealthPlayButton();
    QComboBox* getHealthSelect();
    QSlider* getHealthSlider();
    QLabel* getHealthSliderText();
    QGroupBox* getDeathGroup();
    QPushButton* getDeathPlayButton();
    QComboBox* getDeathSelect();
    QGroupBox* getWebbedGroup();
    QPushButton* getWebbedPlayButton();
    QComboBox* getWebbedSelect();

    /* highlight dialog items */
    QPushButton* getApplyButton();

    void reloadWindowStyles();
    void reloadHighlighterSettings();
    void updateSettings();
    void loadSettings();

private:
    Ui::HighlightDialog *ui;

    MainWindow *mainWindow;
    ClientSettings *settings;
    HighlightGeneralTab *generalTab;
    HighlightTextTab *textTab;
    HighlightAlertTab *alertTab;

public slots:
    void reloadTextHighlights();

private slots:
    void okPressed();
    void cancelPressed();
    void applyPressed();
};

#endif // HIGHLIGHTDIALOG_H
