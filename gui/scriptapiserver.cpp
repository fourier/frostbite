#include "scriptapiserver.h"

#include <QMutexLocker>

#include "gamedatacontainer.h"
#include "apisettings.h"
#include "mainwindow.h"
#include "maps/mapdata.h"
#include "qreadwritelock.h"
#include "tcpclient.h"
#include "gridwindow.h"
#include "windowfacade.h"
#include "window/expwindow.h"
#include "tray.h"
#include "clientsettings.h"
#include "textutils.h"
#include "maps/mapfacade.h"
#include "scriptservice.h"

ScriptApiServer::ScriptApiServer(QObject *parent) : QObject(parent), networkSession(0) {
    mainWindow = (MainWindow*)parent;
    windowFacade = mainWindow->getWindowFacade();

    mapData = mainWindow->getWindowFacade()->getMapFacade()->getData();
    tcpClient = mainWindow->getTcpClient();        

    expWindow = ((GridWindow*)mainWindow->getWindowFacade()->getExpWindow()->getDockWidget()->widget());

    tray = mainWindow->getTray();

    connect(this, SIGNAL(track(QString)), expWindow, SLOT(track(QString)));
    connect(this, SIGNAL(clearTracked()), expWindow, SLOT(clearTracked()));

    connect(this, SIGNAL(windowNames()), windowFacade, SLOT(getStreamWindowNames()));
    connect(this, SIGNAL(addWindow(QString, QString)), windowFacade, SLOT(registerStreamWindow(QString, QString)));
    connect(this, SIGNAL(removeWindow(QString)), windowFacade, SLOT(removeStreamWindow(QString)));
    connect(this, SIGNAL(clearWindow(QString)), windowFacade, SLOT(clearStreamWindow(QString)));
    connect(this, SIGNAL(writeWindow(QString, QString)), windowFacade, SLOT(writeStreamWindow(QString, QString)));
    connect(this, SIGNAL(writeTray(QString, QString)), tray, SLOT(showMessage(QString, QString)));

    data = GameDataContainer::Instance();    

    apiSettings = new ApiSettings();
    clientSettings = ClientSettings::getInstance();

    this->initNetworkSession();
}

void ScriptApiServer::reloadSettings() {
    clientSettings = ClientSettings::getInstance();
    this->openSession();
}

void ScriptApiServer::initNetworkSession() {
    tcpServer = new QTcpServer(this);
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        networkSession = new QNetworkSession(manager.defaultConfiguration(), this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(openSession()));
        networkSession->open();
    } else {
        openSession();
    }
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void ScriptApiServer::openSession() {
    if(tcpServer->isListening()) tcpServer->close();
    if (!tcpServer->listen(QHostAddress::LocalHost, clientSettings->getParameter("Script/apiPort", 0).toInt())) {
        Log4Qt::Logger::logger(QLatin1String("ErrorLogger"))->
                info("Unable to start API server" + tcpServer->errorString());
        return;
    }
    apiSettings->setParameter("ApiServer/port", tcpServer->serverPort());
}

void ScriptApiServer::newConnection() {
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    clientConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void ScriptApiServer::readyRead() {
    QTcpSocket *socket = (QTcpSocket*) sender();
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if(line.startsWith("GET")) {
            ApiRequest request = parseRequest(line.mid(3).trimmed());
            if(request.name == "CHAR_NAME") {
                this->write(socket, tr("%1\\0").arg(data->getCharName()));                                
            } else if(request.name == "EXP_RANK") {
                this->write(socket, tr("%1\\0").arg(data->getExp(request.args.at(0)).value("rank")));
            } else if(request.name == "EXP_STATE") {
                this->write(socket, tr("%1\\0").arg(data->getExp(request.args.at(0)).value("state")));
            } else if(request.name == "ACTIVE_SPELLS") {
                this->write(socket, tr("%1\\0").arg(data->getActiveSpells().join("\n")));
            } else if(request.name == "INVENTORY") {
                this->write(socket, tr("%1\\0").arg(data->getInventory().join("\n")));
            } else if(request.name == "CONTAINER") {
                this->write(socket, tr("%1\\0").arg(data->getContainer().join("\n")));
            } else if(request.name == "WIELD_RIGHT") {
                this->write(socket, tr("%1\\0").arg(data->getRight()));
            } else if(request.name == "WIELD_RIGHT_NOUN") {
                this->write(socket, tr("%1\\0").arg(data->getRightNoun()));
            } else if(request.name == "WIELD_LEFT") {
                this->write(socket, tr("%1\\0").arg(data->getLeft()));
            } else if(request.name == "WIELD_LEFT_NOUN") {
                this->write(socket, tr("%1\\0").arg(data->getLeftNoun()));
            } else if(request.name == "HEALTH") {
                this->write(socket, tr("%1\\0").arg(data->getHealth()));
            } else if(request.name == "CONCENTRATION") {
                this->write(socket, tr("%1\\0").arg(data->getConcentration()));
            } else if(request.name == "SPIRIT") {
                this->write(socket, tr("%1\\0").arg(data->getSpirit()));
            } else if(request.name == "FATIGUE") {
                this->write(socket, tr("%1\\0").arg(data->getFatigue()));
            } else if(request.name == "STANDING") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getStanding())));
            } else if(request.name == "SITTING") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getSitting())));
            } else if(request.name == "KNEELING") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getKneeling())));
            } else if(request.name == "PRONE") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getProne())));
            } else if(request.name == "STUNNED") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getStunned())));
            } else if(request.name == "BLEEDING") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getBleeding())));
            } else if(request.name == "HIDDEN") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getHidden())));
            } else if(request.name == "INVISIBLE") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getInvisible())));
            } else if(request.name == "WEBBED") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getWebbed())));
            } else if(request.name == "JOINED") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getJoined())));
            } else if(request.name == "DEAD") {
                this->write(socket, tr("%1\\0").arg(boolToInt(data->getDead())));
            } else if(request.name == "ROOM_TITLE") {
                this->write(socket, tr("%1\\0").arg(data->getRoomName()));
            } else if(request.name == "ROOM_DESC") {
                this->write(socket, tr("%1\\0").arg(data->getRoomDesc()));
            } else if(request.name == "ROOM_OBJECTS") {
                this->write(socket, tr("%1\\0").arg(data->getRoomObjs()));
            } else if(request.name == "ROOM_PLAYERS") {
                this->write(socket, tr("%1\\0").arg(data->getRoomPlayers()));
            } else if(request.name == "ROOM_EXITS") {
                this->write(socket, tr("%1\\0").arg(data->getRoomExits()));
            } else if(request.name == "RT") {
                this->write(socket, tr("%1\\0").arg(data->getRt()));
            } else if(request.name == "CT") {
                this->write(socket, tr("%1\\0").arg(data->getCt()));
            } else if(request.name == "EXP_NAMES") {
                this->write(socket, tr("%1\\0").arg(data->getExp().keys().join("\n")));
            } else if(request.name == "ROOM_MONSTERS_BOLD") {
                this->write(socket, tr("%1\\0").arg(data->getRoomMonstersBold().join("\n")));
            } else if(request.name == "SCRATCH") {
                QMutexLocker lock(&mutex);
                this->write(socket, tr("%1\\0").arg(scratchText));
            } else {
                this->write(socket, tr("\\0"));
            }
        } else if(line.startsWith("MAP_GET")) {
            ApiRequest request = parseRequest(line.mid(7).trimmed());
            if(request.name == "PATH") {
                QStringList args = request.args;
                if(args.size() < 3) {
                    this->write(socket, tr("\\0"));
                } else {
                    QString path = mapData->findPath(args.at(0), args.at(1).toInt(), args.at(2).toInt());
                    this->write(socket, tr("%1\\0").arg(path));
                }
            } else if(request.name == "CURRENT_ROOM") {
                RoomNode room = mapData->getRoom();
                this->write(socket, QString("{:zone => '%1', :level => %2, :id => %3}\\0")
                            .arg(room.getZoneId(), QString::number(room.getLevel()), QString::number(room.getNodeId())));
            } else if(request.name == "ZONES") {
                this->write(socket, tr("%1\\0").arg(mapData->getZones()));                
            } else if(request.name == "FIND_ROOM") {
                QStringList args = request.args;
                if(args.size() < 1) {
                    this->write(socket, tr("\\0"));
                } else {
                    RoomNode room = mapData->findLocation(args.at(0));
                    this->write(socket, QString("{:zone => '%1', :level => %2, :id => %3}\\0")
                                .arg(room.getZoneId(), QString::number(room.getLevel()), QString::number(room.getNodeId())));
                }
            } else {
                this->write(socket, tr("\\0"));
            }
        } else if(line.startsWith("CLIENT")) {
            ApiRequest request = parseRequest(line.mid(6).trimmed());
            if(request.name == "CONNECT") {
                QStringList args = request.args;
                if(args.size() < 7) {
                    this->write(socket, tr("0\\0"));
                } else {
                    tcpClient->connectApi(args.at(0), args.at(1), args.at(2), args.at(3),
                        args.at(4), args.at(5), TextUtils::toBool(args.at(6)));
                    this->write(socket, tr("1\\0"));
                }
            } else if(request.name == "TRACK_EXP") {
                QStringList args = request.args;
                if(args.size() < 1) {
                    this->write(socket, tr("0\\0"));
                } else {
                    emit track(args.at(0));
                    this->write(socket, tr("1\\0"));
                }
            } else if(request.name == "TRACK_EXP_CLEAR") {
                emit clearTracked();
                this->write(socket, tr("\\0"));
            } else if(request.name == "WINDOW_LIST") {
                QList<QString> list = emit windowNames();
                this->write(socket, tr("%1\\0").arg(list.join("\n")));
            } else if(request.name == "WINDOW_ADD") {
                QStringList args = request.args;
                if(args.size() == 2) {
                    emit addWindow(args.at(0), args.at(1));
                    this->write(socket, tr("1\\0"));
                } else {
                    this->write(socket, tr("0\\0"));
                }
            } else if(request.name == "WINDOW_REMOVE") {
                QStringList args = request.args;
                if(args.size() == 1) {
                    emit removeWindow(args.at(0));
                    this->write(socket, tr("1\\0"));
                } else {
                    this->write(socket, tr("0\\0"));
                }
            } else if(request.name == "WINDOW_CLEAR") {
                QStringList args = request.args;
                if(args.size() == 1) {
                    emit clearWindow(args.at(0));
                    this->write(socket, tr("1\\0"));
                } else {
                    this->write(socket, tr("0\\0"));
                }
            } else if(request.name == "WINDOW_WRITE") {
                QStringList args = request.args;
                if(args.size() == 2) {
                    emit writeWindow(args.at(0), args.at(1));
                    this->write(socket, tr("1\\0"));
                } else {
                    this->write(socket, tr("0\\0"));
                }
            } else if(request.name == "TRAY_WRITE") {
                QStringList args = request.args;
                if(args.size() == 1) {
                    emit writeTray("Script", args.at(0));
                    this->write(socket, tr("1\\0"));
                } else {
                    this->write(socket, tr("0\\0"));
                }
            }
        } else if(line.startsWith("PUT")) {
            ApiRequest request = parseRequest(line.mid(strlen("PUT")).trimmed());
            QString message = request.args.join(" ");
            // send the command to the ScriptService to execute
            // prepending with prefixes as it is coming from the script
            if(request.name == "COMMAND") {
                mainWindow->getScriptService()->processCommand(("put#" + message).toLatin1());
                this->write(socket, tr("1\\0"));
            } else if (request.name == "ECHO") {
                mainWindow->getScriptService()->processCommand(("echo#" + message).toLatin1());
                this->write(socket, tr("1\\0"));
            } else {            // unknown command
                this->write(socket, tr("0\\0"));
            }
        } else {
            this->write(socket, tr("\\0"));
        }
    }
}

void ScriptApiServer::write(QTcpSocket *socket, QString value) {
    socket->write(value.toLocal8Bit());
    socket->flush();
}

ApiRequest ScriptApiServer::parseRequest(QString reqString) {
    ApiRequest apiRequest;

    int index = reqString.indexOf("?");
    if(index > -1) {
        apiRequest.name = reqString.mid(0, index);
        apiRequest.args = reqString.mid(index + 1).split("&");
        for(int i = 0; i < apiRequest.args.size(); i++) {
            apiRequest.args[i] = QUrl::fromPercentEncoding(apiRequest.args[i].toLocal8Bit());
        }
    } else {
        apiRequest.name = reqString;
        apiRequest.args = QStringList();
    }
    return apiRequest;
}

int ScriptApiServer::boolToInt(bool value) {
    if(value) {
        return 1;
    } else {
        return 0;
    }
}

void ScriptApiServer::setScratchText(QString text) {
    QMutexLocker lock(&mutex);
    scratchText = text;    
}

ScriptApiServer::~ScriptApiServer() {
    delete apiSettings;
}



