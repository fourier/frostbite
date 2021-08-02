#include "keyboardfilter.h"
#include "commandline.h"
#include "macrosettings.h"

/* add all keys to map:key, modifier, flags and action text */
KeyboardFilter::KeyboardFilter(QObject *parent) : QObject(parent) {
    macroSettings = MacroSettings::getInstance();
}

void KeyboardFilter::reloadSettings() {
    macroSettings = MacroSettings::getInstance();
}

bool KeyboardFilter::eventFilter(QObject *object, QEvent *event) {
    commandLine = (CommandLine*)object;

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        if(keyEvent->modifiers().testFlag(Qt::KeypadModifier)) {
            #ifdef Q_OS_MAC
            switch(keyEvent->key()) {
                case Qt::Key_Up:
                    commandLine->historyBack();
                    return true;
                break;
                case Qt::Key_Down:
                    commandLine->historyForward();
                    return true;
                break;
            }
            #endif
            QString cmd = macroSettings->getParameter("keypad/" +
                QString::number(keyEvent->modifiers() | keyEvent->key()), "").toString();
            return commandLine->runMacro(cmd);
        } else {
            if(keyEvent->modifiers() == Qt::NoModifier) {
                switch(keyEvent->key()) {
                    case Qt::Key_Up:
                        commandLine->historyBack();
                    break;
                    case Qt::Key_Down:
                        commandLine->historyForward();
                    break;
                    case Qt::Key_Escape:
                        commandLine->clear();
                        commandLine->historyCounter = -1;
                        commandLine->abortScript();
                        commandLine->abortSequence();
                    break;
                    case Qt::Key_Tab:
                        commandLine->completeCommand();
                        return true;
                    break;
                    case Qt::Key_End:
                    case Qt::Key_Home:
                    case Qt::Key_PageUp:
                    case Qt::Key_PageDown:
                        commandLine->windowControl(keyEvent->key());
                    break;
                    default:
                        QString cmd = macroSettings->getParameter("function/" +
                            QString::number(keyEvent->modifiers() | keyEvent->key()), "").toString();
                        return commandLine->runMacro(cmd);
                    break;
                }
                return false;
            } else if(keyEvent->modifiers() == Qt::GroupSwitchModifier) {
              switch (keyEvent->key()) {
                    case Qt::Key_End:
                    case Qt::Key_Home:
                    case Qt::Key_PageUp:
                    case Qt::Key_PageDown:
                        commandLine->windowControl(keyEvent->key());
                    break;
                    default:
                        QString cmd = macroSettings->getParameter("function/" +
                            QString::number(keyEvent->modifiers() | keyEvent->key()), "").toString();
                        return commandLine->runMacro(cmd);
                    break;
              }
              return false;
            } else {
                if(keyEvent->modifiers() == Qt::ControlModifier) {
                    QString cmd = macroSettings->getParameter("ctrl/" +
                        QString::number(keyEvent->modifiers() | keyEvent->key()), "").toString();                    
                    if(cmd.isEmpty() && keyEvent->matches(QKeySequence::Copy)) {
                        commandLine->doCopy();
                        return true;
                    } else {
                        return commandLine->runMacro(cmd);
                    }
                } else if (keyEvent->modifiers() == Qt::AltModifier) {
                    QString cmd = macroSettings->getParameter("alt/" +
                        QString::number(keyEvent->modifiers() | keyEvent->key()), "").toString();
                    return commandLine->runMacro(cmd);
                }
            }
        }
        return false;
    }

    /* workaround to give back focus to
     * command line from alt -> menu */
    if (event->type() == QEvent::FocusOut) {
        commandLine->setFocus();
        return true;
    }

    return false;
}

KeyboardFilter::~KeyboardFilter() {
}
