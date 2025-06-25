#include "SyncsData.h"

SyncsData::SyncsData(QObject* parent):
    QObject(parent)
{}

QString SyncsData::getLocalError() const
{
    return mLocalError;
}

QString SyncsData::getRemoteError() const
{
    return mRemoteError;
}

SyncInfo::SyncOrigin SyncsData::getSyncOrigin() const
{
    return mSyncOrigin;
}

void SyncsData::setLocalError(const QString& error)
{
    if (mLocalError != error)
    {
        mLocalError = error;

        emit localErrorChanged();
    }
}

void SyncsData::setRemoteError(const QString& error)
{
    if (mRemoteError != error)
    {
        mRemoteError = error;

        emit remoteErrorChanged();
    }
}

QString SyncsData::getDefaultLocalFolder() const
{
    return mDefaultLocalFolder;
}

QString SyncsData::getDefaultRemoteFolder() const
{
    return mDefaultRemoteFolder;
}
