#include "EventUpdater.h"

#include <QApplication>

EventUpdater::EventUpdater(int _totalSize, int threshold)
    : mTotalSize(_totalSize)
{
    mUpdateThreshold = (mTotalSize < threshold) ? mTotalSize : threshold;
}

bool EventUpdater::update(int currentSize)
{
    if(mUpdateThreshold > 0)
    {
        if (currentSize % mUpdateThreshold == 0)
        {
            QApplication::processEvents();
            return true;
        }
    }

    return false;
}
