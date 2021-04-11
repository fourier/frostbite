#ifndef ALTER_H
#define ALTER_H

#include <QObject>
#include <QRegularExpression>

#include "altersettingsentry.h"
#include "commandsettingsentry.h"

class IgnoreSettings;
class SubstitutionSettings;
class CommandSettings;

class Alter : public QObject {
    Q_OBJECT

public:
    explicit Alter(QObject *parent = 0);
    ~Alter();

    QString substitute(QString text, const QString& window);
    bool ignore(const QString& text, const QString& window);
    QString command(QString text, const QString& window);
                    
    void reloadSettings();

    QList<AlterSettingsEntry> subsList;
    QList<AlterSettingsEntry> ignoreList;
    QList<CommandSettingsEntry> commandList;

private:
    static QString createUrlCommand(const QString& command);
    
    IgnoreSettings* ignoreSettings;
    SubstitutionSettings* substituteSettings;
    CommandSettings* commandSettings;
    
    bool ignoreEnabled;

signals:

public slots:
};

#endif // ALTER_H
