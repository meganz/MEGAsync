#include "EventUpdater.h"

#include <QApplication>

EventUpdater::EventUpdater(int _totalSize, int threshold)
    : totalSize(_totalSize)
{
    updateThreshold = (totalSize < threshold) ? totalSize : threshold;
}

bool EventUpdater::update(int currentSize)
{
    if (currentSize % updateThreshold == 0)
    {
        QApplication::processEvents();
        return true;
    }

    return false;
}
