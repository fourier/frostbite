#include "commandsettings.h"

#include "clientsettings.h"

#include <QGlobalStatic>

Q_GLOBAL_STATIC(CommandSettingsInstance, uniqueInstance)

CommandSettings* CommandSettings::getInstance() {
    return uniqueInstance;
}

CommandSettings::CommandSettings() {
    clientSettings = ClientSettings::getInstance();
    this->create();
}

void CommandSettings::reInit() {
    QMutexLocker locker(&m_mutex);
    delete settings;
    this->create();
}

void CommandSettings::create() {
    settings = new QSettings(clientSettings->profilePath() + "commands.ini", QSettings::IniFormat);
}

void CommandSettings::setSettings(QList<CommandSettingsEntry> entries) {
    QMutexLocker locker(&m_mutex);
    settings->remove("substitution");
    settings->beginWriteArray("substitution");

    for (int i = 0; i < entries.size(); ++i) {
        CommandSettingsEntry entry = entries.at(i);

        settings->setArrayIndex(i);
        settings->setValue("enabled", entry.enabled);
        settings->setValue("pattern", entry.pattern);
        settings->setValue("command", entry.command);
        settings->setValue("target", entry.targetList);
    }
    settings->endArray();
}

void CommandSettings::addParameter(CommandSettingsEntry entry) {
    QMutexLocker locker(&m_mutex);
    int id = settings->value("substitution/size").toInt();

    settings->beginWriteArray("substitution");
    settings->setArrayIndex(id);
    settings->setValue("enabled", entry.enabled);
    settings->setValue("pattern", entry.pattern);
    settings->setValue("command", entry.command);
    settings->setValue("target", entry.targetList);
    settings->endArray();
}

void CommandSettings::setParameter(CommandSettingsEntry entry) {
    QMutexLocker locker(&m_mutex);
    int size = settings->value("substitution/size").toInt();

    settings->beginWriteArray("substitution");
    settings->setArrayIndex(entry.id);
    settings->setValue("enabled", entry.enabled);
    settings->setValue("pattern", entry.pattern);
    settings->setValue("command", entry.command);
    settings->setValue("target", entry.targetList);
    settings->endArray();

    settings->setValue("substitution/size", size);
}

QList<CommandSettingsEntry> CommandSettings::getCommands() {
    QList<CommandSettingsEntry> settingsCache = QList<CommandSettingsEntry>();
    this->loadSettings("substitution", settingsCache);
    return settingsCache;
}

void CommandSettings::loadSettings(QString group, QList<CommandSettingsEntry> &settingsList) {
    QMutexLocker locker(&m_mutex);
    int size = settings->beginReadArray(group);
    for (int i = 0; i < size; i++) {
        settings->setArrayIndex(i);
        settingsList.append(CommandSettingsEntry((const int&)i,
                (const bool&)settings->value("enabled", "").toBool(),
                (const QString&)settings->value("pattern", "").toString(),
                (const QString&)settings->value("command", "").toString(),
                (const QStringList&)settings->value("target", QStringList()).value<QStringList>()));
    }
    settings->endArray();
}

CommandSettings::~CommandSettings() {
    delete settings;
}
