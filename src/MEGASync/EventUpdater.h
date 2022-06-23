#ifndef EVENTUPDATER_H
#define EVENTUPDATER_H


class EventUpdater
{
public:
    EventUpdater(int _totalSize);
    virtual ~EventUpdater() = default;

    void update(int currentSize);

private:
    int totalSize;
    int updateThreshold;
};

#endif // EVENTUPDATER_H
