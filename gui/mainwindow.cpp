#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cleanlooks/qcleanlooksstyle.h"

#include "windowfacade.h"
#include "tcpclient.h"
#include "toolbar/toolbar.h"
#include "clientsettings.h"
#include "commandline.h"
#include "menuhandler.h"
#include "scriptservice.h"
#include "timerbar.h"
#include "vitalsbar.h"
#include "generalsettings.h"
#include "tray.h"
#include "scriptapiserver.h"
#include "dict/dictionaryservice.h"
#include "text/alter/substitutionsettings.h"
#include "text/alter/ignoresettings.h"
#include "text/highlight/highlightsettings.h"
#include "window/dictionarywindow.h"
#include "maps/mapfacade.h"
#include "maps/mapreader.h"
#include "compass/compassview.h"
#include "macrosettings.h"
#include "hyperlinkservice.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // register types
    qRegisterMetaType<DirectionsList>("DirectionsList");    
    qRegisterMetaType<GridItems>("GridItems");

    // application settings
    this->appSetup();
    // build client content
    this->loadClient();
    // load client settings
    this->initSettings();

    ui->statusBar->hide();
}

void MainWindow::appSetup() {
    // set cleanlooks as base style
    QApplication::setStyle(new QCleanlooksStyle());

    // does not open on correct screen in 4.8
    // https://bugreports.qt-project.org/browse/QTBUG-21371

    // load client window state
    // workaround:
    // https://bugreports.qt.io/browse/QTBUG-16252
    settings = ClientSettings::getInstance();

    if(settings->hasValue("MainWindow/geometry")) {
        QVariant::Type t = settings->getParameter("MainWindow/geometry", "").type();
        if(t == QVariant::Type::Rect) {
            setGeometry(settings->getParameter("MainWindow/geometry", "").toRect());
        } else if(t == QVariant::Type::ByteArray) {
            restoreGeometry(settings->getParameter("MainWindow/geometry", "").toByteArray());
        }
    }
    restoreState(settings->getParameter("MainWindow/state", "").toByteArray());

    // load general settings
    generalSettings = GeneralSettings::getInstance();

    // load tray
    tray = new Tray(this);
}

void MainWindow::toggleFullScreen() {
    setWindowState(Qt::WindowFullScreen);
}

void MainWindow::toggleMaximized() {
    setWindowState(Qt::WindowMaximized);
}

void MainWindow::updateScriptSettings() {
    scriptApiServer->reloadSettings();
}

void MainWindow::menuVolumeChanged(int volume) {
    emit volumeChanged(volume);
}

void MainWindow::menuVolumeMuted(bool muted) {
    emit volumeMuted(muted);
}

void MainWindow::updateProfileSettings(QString name, QString type) {
    settings->setParameter("Profile/name", name);
    settings->setParameter("Profile/type", type);

    GeneralSettings::getInstance()->reInit();
    MacroSettings::getInstance()->reInit();
    SubstitutionSettings::getInstance()->reInit();
    IgnoreSettings::getInstance()->reInit();
    HighlightSettings::getInstance()->reInit();

    emit profileChanged();
}

void MainWindow::reloadSettings() {
    emit profileChanged();
}

void MainWindow::openConnection(QString host, QString port, QString key) {
    tcpClient->connectToHost(host, port, key);
}

void MainWindow::openLocalConnection(QString port) {
    tcpClient->connectToLocalPort(port);
}

MenuHandler* MainWindow::getMenuHandler() {
    return menuHandler;
}

void MainWindow::openConnectDialog() {
    menuHandler->openConnectDialog();
}

void MainWindow::openAppearanceDialog() {
    menuHandler->openAppearanceDialog();
}

void MainWindow::initSettings() {
    /* set focus to command line at startup */
    cmdLine->setFocus();
}

void MainWindow::loadClient() {
    toolBar = new Toolbar(this);
    toolBar->loadToolbar();

    timerBar = new TimerBar(this);
    timerBar->load();

    vitalsBar = new VitalsBar(this);
    vitalsBar->load();

    // add menu items
    this->addWindowMenuAction(ui->mainToolBar->toggleViewAction());
    this->vitalsBar->addToMenu();
    ui->menuWindow->addSeparator();

    windowFacade = new WindowFacade(this);
    windowFacade->loadWindows();

    // add to layout after main window    
    timerBar->add();
    vitalsBar->add();

    cmdLine = new CommandLine(this);
    ui->mainLayout->addWidget(cmdLine);

    scriptService = new ScriptService(this);

    tcpClient = new TcpClient(this);

    menuHandler = new MenuHandler(this);
    menuHandler->loadProfilesMenu();

    scriptApiServer = new ScriptApiServer(this);

    dictionaryService = new DictionaryService(this);

    hyperlinkService = new HyperlinkService(this);
    
    connect(ui->menuBar, SIGNAL(triggered(QAction*)), menuHandler, SLOT(menuTriggered(QAction*)));
    connect(ui->menuBar, SIGNAL(hovered(QAction*)), menuHandler, SLOT(menuHovered(QAction*)));

    connect(dictionaryService, SIGNAL(translationFinished(QString)),
            windowFacade->getDictionaryWindow(), SLOT(write(QString)));
    connect(dictionaryService, SIGNAL(translationFailed(QString)),
            windowFacade->getDictionaryWindow(), SLOT(write(QString)));

    connect(hyperlinkService, SIGNAL(actionCommand(const QString&)),
            this, SLOT(actionCommand(const QString&)));
}

WindowFacade* MainWindow::getWindowFacade() {
    return windowFacade;
}

Toolbar* MainWindow::getToolbar() {
    return toolBar;
}

VitalsBar* MainWindow::getVitalsBar() {
    return vitalsBar;
}

TcpClient* MainWindow::getTcpClient() {
    return tcpClient;
}

CommandLine* MainWindow::getCommandLine() {
    return cmdLine;
}

ScriptService* MainWindow::getScriptService() {
    return scriptService;
}

DictionaryService* MainWindow::getDictionaryService() {
    return dictionaryService;
}

TimerBar* MainWindow::getTimerBar() {
    QReadLocker locker(&lock);
    return this->timerBar;
}

Tray* MainWindow::getTray() {
    return this->tray;
}

void MainWindow::addWidgetMainLayout(QWidget* widget) {
    ui->mainLayout->addWidget(widget);
}

void MainWindow::addDockWidgetMainWindow(Qt::DockWidgetArea area, QDockWidget *dock) {
    QAction *action = dock->toggleViewAction();
    action->setText(action->text());
    ui->menuWindow->addAction(action);

    if(!restoreDockWidget(dock)) {
        this->addDockWidget(area, dock);
    }
}

void MainWindow::removeDockWidgetMainWindow(QDockWidget* dock) {
    this->removeDockWidget(dock);
}

void MainWindow::addWindowMenuAction(QAction* action) {
   ui->menuWindow->addAction(action);
}

QAction* MainWindow::addToolbarWidget(QWidget *widget) {
    return ui->mainToolBar->addWidget(widget);
}

void MainWindow::addToolbarSpacer(int w) {
    QWidget* spacer = new QWidget(ui->mainToolBar);
    spacer->setFixedWidth(w);

    ui->mainToolBar->addWidget(spacer);
}

void MainWindow::addToolbarAction(QAction *action) {
    ui->mainToolBar->addAction(action);
}

void MainWindow::addToolbarSeparator() {
    ui->mainToolBar->addSeparator();
}

void MainWindow::insertProfilesMenu(QMenu* menu) {
    ui->actionLoad_profile->setMenu(menu);    
}

void MainWindow::setLogMain(bool enabled) {    
    ui->actionLogMain->setChecked(enabled);
}

void MainWindow::setLogThoughts(bool enabled) {
    ui->actionLogThoughts->setChecked(enabled);
}

void MainWindow::setLogConversations(bool enabled) {
    ui->actionLogConversations->setChecked(enabled);
}

void MainWindow::setLogArrivals(bool enabled) {
    ui->actionLogArrivals->setChecked(enabled);
}

void MainWindow::setLogDeaths(bool enabled) {
    ui->actionLogDeaths->setChecked(enabled);
}

void MainWindow::setLogDebug(bool enabled) {
    ui->actionLogDebug->setChecked(enabled);
}

void MainWindow::setLogAuth(bool enabled) {
    ui->actionLogAuth->setChecked(enabled);
}

void MainWindow::setWindowLocked(bool enabled) {
    ui->actionLockWindows->setChecked(enabled);
}

void MainWindow::setMenuWieldLeftVisible(bool enabled) {
    ui->actionToolWieldLeft->setChecked(enabled);
}

void MainWindow::setMenuWieldRightVisible(bool enabled) {
    ui->actionToolWieldRight->setChecked(enabled);
}

void MainWindow::setMenuSpellVisible(bool enabled) {
    ui->actionToolSpell->setChecked(enabled);
}

void MainWindow::setMenuActiveSpellsVisible(bool enabled) {
    ui->actionToolActiveSpells->setChecked(enabled);
}

void MainWindow::setMenuStatusVisible(bool enabled) {
    ui->actionToolStatus->setChecked(enabled);
}

void MainWindow::setMenuButtonsVisible(bool enabled) {
    ui->actionToolButtons->setChecked(enabled);
}

void MainWindow::setMenuVitalsVisible(bool enabled) {
    ui->actionToolVitals->setChecked(enabled);
}

void MainWindow::setMenuMutedVisible(bool enabled) {
    ui->actionToolMute->setChecked(enabled);
}

void MainWindow::setToolbarAllowedAreas(Qt::ToolBarAreas areas) {
    ui->mainToolBar->setAllowedAreas(areas);
}

void MainWindow::reloadMaps() {
    this->windowFacade->getMapFacade()->getMapReader()->reload();
}

void MainWindow::setCompassLocked(bool checked) {
    ui->actionLockCompass->setChecked(checked);
}

void MainWindow::setCompassAnchored(bool checked) {
    ui->actionCompassBottomRight->setChecked(checked);
}

void MainWindow::setCompassVisible(bool checked) {
    ui->actionCompassVisible->setChecked(checked);
}

void MainWindow::enableMapsMenu(bool enabled) {
    QMenu* menu = ui->menuBar->findChild<QMenu*>("menuMap");
    if(menu != NULL)menu->setEnabled(enabled);
}

void MainWindow::showMaps() {
    this->windowFacade->getMapFacade()->showMapDialog();
}

void MainWindow::setMainTitle(QString roomName) {
    setWindowTitle("The Frostbite Client" + roomName);
}

void MainWindow::connectEnabled(bool enabled) {
    ui->actionConnect->setEnabled(enabled);
}

void MainWindow::handleAppMessage(const QString& msg) {
    if(msg == "show") {
        this->raise();
        this->activateWindow();
    }
}

void MainWindow::saveWindow() {
    settings->setParameter("MainWindow/state", saveState());
    settings->setParameter("MainWindow/geometry", QVariant(geometry()));
    settings->setParameter("Compass/geometry", QVariant(windowFacade->getCompassView()->geometry()));
}

void MainWindow::closeEvent(QCloseEvent*) {
    /* save client window state */
    this->saveWindow();

    /* terminate if script running at exit */
    scriptService->terminateScript();
}

void MainWindow::actionCommand(const QString& command) {
    getCommandLine()->setText(command);
    emit getCommandLine()->sendCommand();
}

MainWindow::~MainWindow() {
    delete tcpClient;
    delete ui;
    delete toolBar;
    delete windowFacade;
    delete cmdLine;
    delete menuHandler;
    delete timerBar;
}
