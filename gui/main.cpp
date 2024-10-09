#include <guiapplication.h>
#include "mainwindow.h"

#include "log4qt/logger.h"
#include <log4qt/propertyconfigurator.h>

#ifdef __linux__
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#endif

bool MainWindow::DEBUG = false;

#ifdef __linux__
void handler(int sig) {
    void* array[50];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 50);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

int main(int argc, char* argv[]) {
    QCoreApplication::setApplicationVersion(QString(RELEASE_VERSION));
    QCoreApplication::setApplicationName("Frostbite");

    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO)
        && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
        && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
        && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }

#ifdef __linux__
    signal(SIGSEGV, handler);
#endif

    GuiApplication a(argc, argv);

    /* Prohibit running more than one copy of appliction. */
    if (a.isRunning()) {
        Log4Qt::Logger::logger(QLatin1String("ErrorLogger"))->info("Application already running.");
        a.sendMessage("show", 1000);
        exit(0);
    }

    Log4Qt::PropertyConfigurator::configure(QApplication::applicationDirPath() + "/log.ini");

    QApplication::addLibraryPath(QApplication::applicationDirPath());

    MainWindow w;
    w.show();

    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w,
                     SLOT(handleAppMessage(const QString&)));

    QStringList args = QCoreApplication::arguments();
    if (!args.isEmpty() && args.count() > 1) {
        if (args.at(1).startsWith("--port=")) {
            w.openLocalConnection(args.at(1).mid(7).trimmed());
        } else if (QFile(args.at(1)).exists()) {
            QSettings settings(args.at(1), QSettings::IniFormat);
            if (settings.contains("GAMEHOST") && settings.contains("GAMEPORT")
                && settings.contains("KEY")) {
                w.openConnection(settings.value("GAMEHOST").toString(),
                                 settings.value("GAMEPORT").toString(),
                                 settings.value("KEY").toString());
            }
            return a.exec();
        }
    }

    if (!MainWindow::DEBUG) {
        w.openConnectDialog();
    }
    return a.exec();
}
