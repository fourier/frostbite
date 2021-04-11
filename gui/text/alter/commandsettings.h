#ifndef COMMANDSETTINGS_H
#define COMMANDSETTINGS_H

#include <QMutex>
#include <QSettings>

#include "text/alter/commandsettingsentry.h"

class ClientSettings;

class CommandSettings {
    friend class CommandSettingsInstance;

public:
    static CommandSettings* getInstance();
    ~CommandSettings();

    void reInit();

    void setParameter(CommandSettingsEntry entry);
    void addParameter(CommandSettingsEntry entry);
    QList<CommandSettingsEntry> getCommands();
    void setSettings(QList<CommandSettingsEntry> entries);
    void loadSettings(QString, QList<CommandSettingsEntry>&);

private:
    explicit CommandSettings();

    void create();

    QSettings* settings;
    ClientSettings* clientSettings;

    //QReadWriteLock lock;
    QMutex m_mutex;

signals:

public slots:

};

class CommandSettingsInstance : public CommandSettings {
};

#endif // COMMANDSETTINGS_H
