#include "debuglogger.h"

DebugLogger::DebugLogger(QObject *parent) {
}

void DebugLogger::addText(QString text) {
    mMutex.lock();
    dataQueue.enqueue(text);
    mMutex.unlock();
}

void DebugLogger::run() {
    while(!dataQueue.isEmpty()) {
        mMutex.lock();
        localData = dataQueue.dequeue();
        mMutex.unlock();

        logger()->info(localData + "\n");
    }
}

DebugLogger::~DebugLogger() {
}