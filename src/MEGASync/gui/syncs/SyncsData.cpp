#include "SyncsData.h"

namespace
{
const QString DEFAULT_MEGA_FOLDER = QString::fromUtf8("MEGA");
const QString DEFAULT_MEGA_PATH = QString::fromUtf8("/") + DEFAULT_MEGA_FOLDER;
}

SyncsData::SyncsData(const Syncs* syncs):
    mSyncs(syncs)
{}

QString SyncsData::getDefaultMegaFolder() const
{
    return Syncs::getDefaultMegaFolder();
}

QString SyncsData::getDefaultMegaPath() const
{
    return Syncs::getDefaultMegaPath();
}

Syncs::SyncStatusCode SyncsData::getSyncStatus() const
{
    return mSyncs->getSyncStatus();
}

QString SyncsData::getLocalError() const
{
    return mSyncs->getLocalError();
}

QString SyncsData::getRemoteError() const
{
    return mSyncs->getRemoteError();
}
