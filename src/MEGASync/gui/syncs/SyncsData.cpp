#include "SyncsData.h"

#include "Syncs.h"

#include <cassert>

namespace
{
const QString DEFAULT_MEGA_FOLDER = QString::fromUtf8("MEGA");
const QString DEFAULT_MEGA_PATH = QString::fromUtf8("/") + DEFAULT_MEGA_FOLDER;
}

SyncsData::SyncsData(const Syncs* syncs):
    mSyncs(syncs)
{}

QString SyncsData::getDefaultMegaFolder()
{
    return DEFAULT_MEGA_FOLDER;
}

QString SyncsData::getDefaultMegaPath()
{
    return DEFAULT_MEGA_PATH;
}

SyncsUtils::SyncStatusCode SyncsData::getSyncStatus() const
{
    assert(mSyncs != nullptr);
    return mSyncs->getSyncStatus();
}

QString SyncsData::getLocalError() const
{
    assert(mSyncs != nullptr);
    return mSyncs->getLocalError();
}

QString SyncsData::getRemoteError() const
{
    assert(mSyncs != nullptr);
    return mSyncs->getRemoteError();
}
