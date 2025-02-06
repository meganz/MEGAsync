#ifndef DRIVEDATA_H
#define DRIVEDATA_H

#include <QObject>

class DriveSpaceData
{
public:
    DriveSpaceData();

    bool isAvailable() const;

    bool mIsReady;

    int64_t mAvailableSpace;
    int64_t mTotalSpace;
};

#endif // DRIVEDATA_H
