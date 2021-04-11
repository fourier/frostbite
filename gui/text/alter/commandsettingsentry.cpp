#include "commandsettingsentry.h"

CommandSettingsEntry::CommandSettingsEntry() {

}

CommandSettingsEntry::CommandSettingsEntry(const int& id, const bool& enabled, const QString& pattern,
        const QStringList& targetList) {
    this->id = id;
    this->enabled = enabled;
    this->pattern = pattern;    

    this->targetList = targetList;
}

CommandSettingsEntry::CommandSettingsEntry(const int& id, const bool& enabled, const QString& pattern,
        const QString& command, const QStringList& targetList) {
    this->id = id;
    this->enabled = enabled;
    this->pattern = pattern;
    this->command = command;
    this->targetList = targetList;
}

const QString CommandSettingsEntry::toString() {
    return "CommandSettingsEntry:[ id => " + QString::number(this->id) +
            ", pattern => " + this->pattern +
            ", command => " + this->command + " ]";
}
