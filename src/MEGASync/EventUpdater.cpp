#include "EventUpdater.h"

#include <QApplication>

EventUpdater::EventUpdater(int _totalSize)
    : totalSize(_totalSize)
{
    const int maxItemsBeforeUpdate = 500;
    updateThreshold = (totalSize < maxItemsBeforeUpdate) ? totalSize : maxItemsBeforeUpdate;
}

void EventUpdater::update(int currentSize)
{
    if (currentSize % updateThreshold == 0)
    {
        QApplication::processEvents();
    }
}
