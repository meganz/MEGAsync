
#include "SyncSettings.h"
#include "platform/Platform.h"

#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif


SyncSetting::~SyncSetting()
{
}

QString SyncSetting::getSyncID() const
{
    return mSyncID;
}

void SyncSetting::setSyncID(const QString &syncID)
{
    mSyncID = syncID;
}

SyncSetting::SyncSetting()
{
}

SyncSetting::SyncSetting(MegaSync *sync)
{
    setSync(sync);
}

QString SyncSetting::name() const
{
    return mName;
}

void SyncSetting::setEnabled(bool value)
{
    mEnabled = value;
}

SyncSetting::SyncSetting(QString initializer)
{
    QStringList parts = initializer.split(QString::fromUtf8("0x1E"));
    int i = 0;
    if (i<parts.size()) { mTag = parts.at(i++).toInt(); }
    if (i<parts.size()) { mSyncID = parts.at(i++); }

    if (i<parts.size()) { mEnabled = parts.at(i++).toInt(); }
    if (i<parts.size()) { mLocalFolder = parts.at(i++); }
    if (i<parts.size()) { mName = parts.at(i++); }
    if (i<parts.size()) { mLocalFingerPrint = parts.at(i++).toLongLong(); }
    if (i<parts.size()) { mMegaFolder = parts.at(i++); }
    if (i<parts.size()) { mMegaHandle = parts.at(i++).toLongLong(); }
    if (i<parts.size()) { mActive = parts.at(i++).toInt(); }
    if (i<parts.size()) { mTemporaryDisabled = parts.at(i++).toInt(); }
    if (i<parts.size()) { mError = parts.at(i++).toInt(); }
}


SyncSetting::SyncSetting(const SyncData &osd, bool loadedFromPreviousSessions)
{
    mSyncID = osd.mSyncID;
    mEnabled = osd.mEnabled;
    mLocalFolder = osd.mLocalFolder;
    mName = osd.mName;
    mLocalFingerPrint = osd.mLocalfp;
    mMegaFolder = osd.mMegaFolder;
    mMegaHandle = osd.mMegaHandle;

    // mActive should be false for loadedFromPreviousSessions
    mActive = !loadedFromPreviousSessions && osd.mEnabled && !osd.mTemporarilyDisabled;

    mTemporaryDisabled = osd.mTemporarilyDisabled;
    mError = osd.mSyncError;
}


QString SyncSetting::toString()
{
    QStringList toret;
    toret.append(QString::number(mTag));
    toret.append(mSyncID);

    toret.append(QString::number(mEnabled));
    toret.append(mLocalFolder);
    toret.append(mName);
    toret.append(QString::number(mLocalFingerPrint));
    toret.append(mMegaFolder);
    toret.append(QString::number(mMegaHandle));
    toret.append(QString::number(mActive));
    toret.append(QString::number(mTemporaryDisabled));
    toret.append(QString::number(mError));

    return toret.join(QString::fromUtf8("0x1E"));
}

void SyncSetting::setSync(MegaSync *sync)
{
    if (sync)
    {
        assert(mTag == 0 || mTag == sync->getTag());
        mTag = sync->getTag();
        mEnabled = sync->isEnabled();

        mLocalFolder = QString::fromUtf8(sync->getLocalFolder());
        mName = QString::fromUtf8(sync->getName());
        mLocalFingerPrint = sync->getLocalFingerprint();

        auto megafolder = sync->getMegaFolder();
        mMegaFolder = megafolder ? QString::fromUtf8(megafolder) : QString();

        mMegaHandle = sync->getMegaHandle();
        mActive = sync->isActive();
        mState = sync->getState();
        mTemporaryDisabled = sync->isTemporaryDisabled();
        mError = sync->getError();
    }
    else
    {
        assert("SyncSettings constructor with null sync");
    }
}

QString SyncSetting::getLocalFolder() const
{
    return mLocalFolder;
}

long long SyncSetting::getLocalFingerprint()  const
{
    return mLocalFingerPrint;
}

QString SyncSetting::getMegaFolder()  const
{
    return mMegaFolder;
}

long long SyncSetting::getMegaHandle()  const
{
    return mMegaHandle;
}

bool SyncSetting::isEnabled()  const
{
    return mEnabled;
}

bool SyncSetting::isActive()  const
{
    return mActive;
}

int SyncSetting::getState() const
{
    return mState;
}

bool SyncSetting::isTemporaryDisabled()  const
{
    return mTemporaryDisabled;
}

int SyncSetting::getError() const
{
    return mError;
}

int SyncSetting::tag() const
{
    return mTag;
}

void SyncSetting::setTag(int tag)
{
    mTag = tag;
}
