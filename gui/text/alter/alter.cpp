#include "alter.h"

#include <QUrl>

#include "text/alter/substitutionsettings.h"
#include "text/alter/ignoresettings.h"
#include "text/alter/commandsettings.h"
#include "textutils.h"
#include "globaldefines.h"

Alter::Alter(QObject *parent) : QObject(parent) {
    ignoreSettings = IgnoreSettings::getInstance();
    ignoreList = ignoreSettings->getIgnores();
    substituteSettings = SubstitutionSettings::getInstance();
    subsList = substituteSettings->getSubstitutions();
    commandSettings = CommandSettings::getInstance();
    commandList = commandSettings->getCommands();
    ignoreEnabled = ignoreSettings->getEnabled();
}

void Alter::reloadSettings() {
    ignoreSettings->reInit();
    ignoreList = ignoreSettings->getIgnores();
    substituteSettings->reInit();
    subsList = substituteSettings->getSubstitutions();
    ignoreEnabled = ignoreSettings->getEnabled();
    commandSettings->reInit();
    commandList = commandSettings->getCommands();
}

QString Alter::substitute(QString text, const QString& window) {
    if(!text.isEmpty()) {        
        for(const auto& entry : subsList) {
            if(!entry.enabled || entry.pattern.isEmpty()) continue;
            if(!entry.targetList.empty() && !entry.targetList.contains(window)) continue;
            text.replace(QRegularExpression(entry.pattern + "(?=[^>]*(<|$))"), entry.substitute);
        }
    }
    return text;
}

bool Alter::ignore(const QString& text, const QString& window) {
    if(!ignoreEnabled) return false;
    for(const auto& entry : ignoreList) {
        if(!entry.enabled || entry.pattern.isEmpty()) continue;
        if(!entry.targetList.empty() && !entry.targetList.contains(window)) continue;
        if (QRegularExpression(entry.pattern + "(?=[^>]*(<|$))").match(text).hasMatch()) {
           return true;
        }
    }
    return false;
}

QString Alter::command(QString text, const QString &window) {
    if(!text.isEmpty()) {        
        for(const auto& entry : commandList) {
            if(!entry.enabled || entry.pattern.isEmpty()) continue;
            if(!entry.targetList.empty() && !entry.targetList.contains(window)) continue;
            text.replace(QRegularExpression(entry.pattern + "(?=[^>]*(<|$))"), createUrlCommand(entry.command));
        }
    }
    return text;
}

QString Alter::createUrlCommand(const QString& command) {
    auto cmd = QString("<a href=\"") + FROSTBITE_SCHEMA + QString("://action/") + command + "\">\\1</a>";
    return cmd;
}


Alter::~Alter() {
}
