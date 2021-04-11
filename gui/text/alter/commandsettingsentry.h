#ifndef COMMANDSETTINGSENTRY_H
#define COMMANDSETTINGSENTRY_H

#include <QList>

class CommandSettingsEntry {

public:
    CommandSettingsEntry();

    CommandSettingsEntry(const int& id, const bool& enabled, const QString& pattern, const QStringList& targetList);

    CommandSettingsEntry(const int& id, const bool& enabled, const QString& pattern, const QString& command, const QStringList& targetList);

    const QString toString();

    int id;
    bool enabled;
    QString pattern;
    QString command;
    QStringList targetList;
};

//typedef QList<CommandSettingsEntry> commandSettingsEntryList;

#endif // COMMANDSETTINGSENTRY_H
