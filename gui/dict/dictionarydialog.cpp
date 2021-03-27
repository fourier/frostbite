#include "dictionarydialog.h"
#include "dictionarysettings.h"
#include "qnamespace.h"

#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QRadioButton>

#include <algorithm>

namespace {
struct FindModifier {
    FindModifier(const Qt::KeyboardModifier mod) : modifier(mod) {}
    bool operator()(const DictionaryDialog::ButtonPair& pair) {
        return pair.first == modifier;
    }
    Qt::KeyboardModifier modifier;
};
} // anonymous


DictionaryDialog::DictionaryDialog(QWidget *parent) : QDialog(parent), settings(DictionarySettings::getInstance()) {
    setWindowTitle(tr("Dictionary settings"));

    QPushButton *okButton = new QPushButton(tr("Ok"), this);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    
    connect(okButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    dictNameEdit = new QLineEdit("", this);
    dictArgumentsEdit = new QLineEdit("", this);

    hotkeyOptionsBox = new QGroupBox(tr("Enable Double-click to translate"), this);
    hotkeyOptionsBox->setCheckable(true);
    hotkeyOptionsBox->setChecked(false);

    QVBoxLayout *vbox = new QVBoxLayout;
    dblClkButtons.push_back(std::make_pair(Qt::NoModifier, new QRadioButton(tr("Double click"))));
    dblClkButtons.push_back(std::make_pair(Qt::AltModifier, new QRadioButton(tr("Alt + Double click"))));
    dblClkButtons.push_back(std::make_pair(Qt::ControlModifier, new QRadioButton(tr("Ctrl + Double click"))));
    dblClkButtons.push_back(std::make_pair(Qt::ShiftModifier, new QRadioButton(tr("Shift + Double click"))));
    for (size_t i = 0; i < dblClkButtons.size(); ++ i) {
        vbox->addWidget(dblClkButtons[i].second);
    }
    vbox->addStretch(1);
    hotkeyOptionsBox->setLayout(vbox);
    dblClkButtons[0].second->setChecked(true);
    loadSettings();
    
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(new QLabel(tr("Dictionary program name:")), 0, 0);
    mainLayout->addWidget(dictNameEdit, 0, 1, 1, 3);
    mainLayout->addWidget(new QLabel(tr("Command line arguments:")), 1, 0);
    mainLayout->addWidget(dictArgumentsEdit, 1, 1, 1, 3);
    mainLayout->addWidget(hotkeyOptionsBox, 2, 0, 1, 4);
    mainLayout->addWidget(okButton, 3, 2);
    mainLayout->addWidget(cancelButton, 3, 3);    
}

DictionaryDialog::~DictionaryDialog() {
}

void DictionaryDialog::okPressed() {
    this->saveSettings();
    this->loadSettings();
    this->accept();
}

void DictionaryDialog::cancelPressed() {
    this->loadSettings();
    this->reject();
}

void DictionaryDialog::loadSettings() {
    dictNameEdit->setText(settings->getDictCommand());
    dictArgumentsEdit->setText(settings->getDictArguments());
    hotkeyOptionsBox->setChecked(settings->getDoubleClickEnabled());
    
    ButtonVector::iterator found = std::find_if(dblClkButtons.begin(),
                                                dblClkButtons.end(),
                                                FindModifier(settings->getDoubleClickModifier()));
    if (found != dblClkButtons.end()) {
        found->second->setChecked(true);
    }
}


void DictionaryDialog::saveSettings() {
    Qt::KeyboardModifier modifier = Qt::NoModifier;
    for (size_t i = 0; i < dblClkButtons.size(); ++ i) {
        if (dblClkButtons[i].second->isChecked()) {
            modifier = dblClkButtons[i].first;
        }
    }

    settings->setDictCommand(dictNameEdit->text())
        .setDictArguments(dictArgumentsEdit->text())
        .setDoubleClickEnabled(hotkeyOptionsBox->isChecked())
        .setDoubleClickModifier(modifier);
}