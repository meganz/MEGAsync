#ifndef DRIVEDATA_H
#define DRIVEDATA_H

#include <QObject>

class DriveSpaceData
{
public:
    DriveSpaceData();

    bool isAvailable() const;

    bool mIsReady;
    qint64 mAvailableSpace;
    qint64 mTotalSpace;
};

struct DriveDisplayData
{
    QString name;
    QString icon;
};

#endif // DRIVEDATA_H
