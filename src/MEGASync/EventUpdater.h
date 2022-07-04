#ifndef EVENTUPDATER_H
#define EVENTUPDATER_H


class EventUpdater
{
public:
    EventUpdater(int _totalSize, int threshold = 100);
    virtual ~EventUpdater() = default;

    bool update(int currentSize);

private:
    int mTotalSize;
    int mUpdateThreshold;
};

#endif // EVENTUPDATER_H
