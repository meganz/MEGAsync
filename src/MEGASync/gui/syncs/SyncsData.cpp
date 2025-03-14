#include "SyncsData.h"

#include "Syncs.h"

#include <cassert>

namespace
{
const QString DEFAULT_MEGA_FOLDER = QString::fromUtf8("MEGA");
const QString DEFAULT_MEGA_PATH = QString::fromUtf8("/") + DEFAULT_MEGA_FOLDER;
}

SyncsData::SyncsData(QObject* parent):
    QObject(parent)
{}

QString SyncsData::getDefaultMegaFolder()
{
    return DEFAULT_MEGA_FOLDER;
}

QString SyncsData::getDefaultMegaPath()
{
    return DEFAULT_MEGA_PATH;
}

QString SyncsData::getLocalError() const
{
    return mLocalError;
}

QString SyncsData::getRemoteError() const
{
    return mRemoteError;
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
