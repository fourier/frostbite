#include "gridwindow.h"

GridWindow::GridWindow(QString title, QWidget *parent) : QTableWidget(parent) {
    mainWindow = (MainWindow*)parent;
    settings = GeneralSettings::getInstance();
    wm = mainWindow->getWindowFacade();

    this->windowId = title.simplified().remove(' ') + "Window";

    this->buildContextMenu();
    this->loadSettings();    

    connect(this, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(addRemoveTracked(int, int)));
    connect(this->verticalHeader(), SIGNAL(sectionCountChanged(int, int)), this, SLOT(resize(int, int)));
    connect(mainWindow->getWindowFacade(), SIGNAL(updateWindowSettings()), this, SLOT(updateSettings()));

    this->setFocusPolicy(Qt::NoFocus);
}

void GridWindow::updateSettings() {    
    settings = GeneralSettings::getInstance();
    this->loadSettings();
    this->clearTracked();
    this->updateFont();
}

void GridWindow::loadSettings() {        
    QVariant fontValue = settings->getParameter(windowId + "/font", QVariant());
    if(!fontValue.isNull()) {
       font = fontValue.value<QFont>();
       this->setProperty(WINDOW_FONT_ID, font);
       fontAct->setText(font.family() + " " + QString::number(font.pointSize()));
    } else {
       font = settings->dockWindowFont();
       this->setProperty(WINDOW_FONT_ID, QVariant());
       fontAct->setText(WINDOW_FONT_SET);
    }
    font.setStyleStrategy(QFont::PreferAntialias);
    this->setFont(font);

    textColor = settings->dockWindowFontColor();
    backgroundColor = settings->dockWindowBackground();
}

void GridWindow::resize(int, int) {
    this->resizeRowsToContents();
    int height = this->verticalHeader()->fontMetrics().height();
    this->verticalHeader()->setDefaultSectionSize(height + 4);
}

QColor GridWindow::getBgColor() {
    return viewport()->palette().color(QPalette::Base);
}

QColor GridWindow::getTextColor() {
    return viewport()->palette().color(QPalette::Text);
}

QLabel* GridWindow::gridValueLabel(QWidget* parent) {
    QLabel* label = new QLabel(parent);    
    label->setFont(font);

    QPalette p = label->palette();
    p.setColor(QPalette::Text, textColor);
    p.setColor(QPalette::Base, backgroundColor);
    label->setPalette(p);

    label->setTextFormat(Qt::RichText);
    label->setAutoFillBackground(true);
    label->setProperty("tracked", 0);
    return label;
}

void GridWindow::invertColors(QWidget* widget) {
    QPalette p = widget->palette();
    QRgb rgbText = p.color(QPalette::Text).rgba()^0xffffff;
    QRgb rgbBase = p.color(QPalette::Base).rgba()^0xffffff;

    p.setColor(QPalette::Text, QColor(rgbText));
    p.setColor(QPalette::Base, QColor(rgbBase));

    widget->setPalette(p);
}

void GridWindow::setItemColors(QWidget* widget, QColor text, QColor background) {
    QPalette p = widget->palette();
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Base, background);
    widget->setPalette(p);
}

void GridWindow::track(QString skillName) {
    if(!tracked.contains(skillName))
        tracked << skillName;

    int rows = this->rowCount();

    QRegExp rx(skillName);
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    for(int i = 0; i < rows; i++) {
        QWidget* widget = this->cellWidget(i, 0);
        if(widget != NULL) {
            if(rx.exactMatch(widget->objectName())) track(skillName, widget);
        }
    }
}

void GridWindow::track(QString skillName, QWidget* widget) {
    if(tracked.contains(skillName)) {
        if(widget->property("tracked") == 0) {
            this->setItemColors(widget, backgroundColor, textColor);
            widget->setProperty("tracked", 1);
        }
    } else {
        if(widget->property("tracked") == 1) {
            this->setItemColors(widget, textColor, backgroundColor);
            widget->setProperty("tracked", 0);
        }
    }
}

void GridWindow::addRemoveTracked(int row, int col) {
    QWidget* w = this->cellWidget(row, col);
    if(!tracked.contains(w->objectName())) {
        tracked << w->objectName();
    } else {
        tracked.removeAll(w->objectName());
    }
    this->track(w->objectName(), w);
}

void GridWindow::clearTracked() {
    tracked.clear();    
    for (int i = 0; i < this->rowCount(); ++i) {
        QWidget* w = this->cellWidget(i, 0);
        this->setItemColors(w, textColor, backgroundColor);
        w->setProperty("tracked", 0);
    }
}

void GridWindow::contextMenuEvent(QContextMenuEvent* event) {
    menu->exec(event->globalPos());
}

void GridWindow::buildContextMenu() {
    menu = new QMenu(this);

    appearanceAct = new QAction(tr("&Appearance\t"), this);
    menu->addAction(appearanceAct);
    connect(appearanceAct, SIGNAL(triggered()), this, SLOT(changeAppearance()));

    menu->addSeparator();

    fontAct = new QAction(tr("&" WINDOW_FONT_SET "\t"), this);
    QVariant fontValue = settings->getParameter(windowId + "/font", QVariant());
    if(!fontValue.isNull()) {
        QFont font = fontValue.value<QFont>();
        fontAct->setText(font.family() + " " + QString::number(font.pointSize()));
    }

    menu->addAction(fontAct);
    connect(fontAct, SIGNAL(triggered()), this, SLOT(selectFont()));

    clearFontAct = new QAction(tr("&" WINDOW_FONT_CLEAR "\t"), this);
    menu->addAction(clearFontAct);
    connect(clearFontAct, SIGNAL(triggered()), this, SLOT(clearFont()));
}

void GridWindow::selectFont() {
    bool ok;
    QFont windowFont = QFontDialog::getFont(&ok, font, this);
    if(ok) {
        fontAct->setText(windowFont.family() + " " + QString::number(windowFont.pointSize()));
        settings->setParameter(windowId + "/font", windowFont);
        font = windowFont;
        this->setProperty(WINDOW_FONT_ID, windowFont);
        this->setFont(windowFont);
        this->updateFont();
    }
}

void GridWindow::clearFont() {
    QFont windowFont = settings->getParameter("DockWindow/font",
        QFont(DEFAULT_DOCK_FONT, DEFAULT_DOCK_FONT_SIZE)).value<QFont>();
    font = windowFont;
    this->setFont(windowFont);
    fontAct->setText(WINDOW_FONT_SET);
    settings->setParameter(windowId + "/font", QVariant());
    this->setProperty(WINDOW_FONT_ID, QVariant());
    this->updateFont();
}

void GridWindow::updateFont() {
    for(int i = 0; i < this->rowCount(); i++) {
        QWidget* widget = this->cellWidget(i, 0);
        if(widget != NULL) {
            ((QLabel*)widget)->setFont(font);
        }
    }
}

void GridWindow::changeAppearance() {
    mainWindow->openAppearanceDialog();
}

GridWindow::~GridWindow() {
    delete fontAct;
    delete clearFontAct;
    delete menu;
}
