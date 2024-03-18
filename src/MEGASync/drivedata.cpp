#include "drivedata.h"

DriveSpaceData::DriveSpaceData()
{

}

bool DriveSpaceData::isAvailable() const
{
    return mIsReady && mTotalSpace > 0;
}
