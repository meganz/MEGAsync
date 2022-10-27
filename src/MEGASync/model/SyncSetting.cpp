
#include "SyncSetting.h"
#include "platform/Platform.h"

#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif


SyncSetting::~SyncSetting()
{
}

SyncSetting::SyncSetting(const SyncSetting& a) :
    mSync(a.getSync()->copy()), mBackupId(a.backupId()),
    mSyncID(a.getSyncID()), mEnabled(a.isEnabled()),
    mActive(a.isActive()), mMegaFolder(a.mMegaFolder)
{
}

SyncSetting& SyncSetting::operator=(const SyncSetting& a)
{
    mSync.reset(a.getSync()->copy());
    mBackupId = a.backupId();
    mSyncID = a.getSyncID();
    mEnabled = a.isEnabled();
    mActive = a.isActive();
    mMegaFolder = a.mMegaFolder;
    return *this;
}

QString SyncSetting::getSyncID() const
{
    return mSyncID;
}

void SyncSetting::setSyncID(const QString &syncID)
{
    mSyncID = syncID;
}

void SyncSetting::setMegaFolder(const QString &megaFolder)
{
    mMegaFolder = megaFolder;
}

SyncSetting::SyncSetting()
{
    mSync.reset(new MegaSync()); // MegaSync getters return fair enough defaults
}

SyncSetting::SyncSetting(MegaSync *sync)
{
    setSync(sync);
}

QString SyncSetting::name(bool removeUnsupportedChars) const
{
    //Provide name removing ':' to avoid possible issues during communications with shell extension
    return removeUnsupportedChars ? QString::fromUtf8(mSync->getName()).remove(QChar::fromAscii(':'))
                                  : QString::fromUtf8(mSync->getName());
}

void SyncSetting::setEnabled(bool value)
{
    mEnabled = value;
}

MegaSync * SyncSetting::getSync() const
{
    return mSync.get();
}

SyncSetting::SyncSetting(QString initializer)
{
    if (!initializer.isEmpty())
    {
        QStringList parts = initializer.split(QString::fromUtf8("0x1E"));
        int i = 0;
        int cacheVersion = 0;
        if (i<parts.size()) { cacheVersion = parts.at(i++).toInt(); }
        if (cacheVersion >= 1)
        {
            if (i<parts.size()) { mBackupId = parts.at(i++).toULongLong(); }
            if (i<parts.size()) { mSyncID = parts.at(i++); }
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Unexpected SyncSetting cache version: %1")
                     .arg(cacheVersion).toUtf8().constData());
        }
    }
    else
    {
        assert(false && "unexpected empty initializer");
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Unexpected SyncSetting empty initializer").toUtf8().constData());
    }

    mSync.reset(new MegaSync()); // MegaSync getters return fair enough defaults
}

SyncSetting::SyncSetting(const SyncData &osd, bool/* loadedFromPreviousSessions*/)
{
    mSyncID = osd.mSyncID;
    mEnabled = osd.mEnabled;

    // Although mActive should be false for loadedFromPreviousSessions, since MEGAsync versions with old cache
    // did not deActivate syncs when logging out, we dont need to consider this
    // keeping the parameter and the code in case we consider fixing that. Uncoment the /**/ in that case.
    mActive = /*!loadedFromPreviousSessions && */osd.mEnabled && !osd.mTemporarilyDisabled;

}

QString SyncSetting::toString()
{
    QStringList toret;
    toret.append(QString::number(CACHE_VERSION));
    toret.append(QString::number(mBackupId));
    toret.append(mSyncID);

    return toret.join(QString::fromUtf8("0x1E"));
}

void SyncSetting::setSync(MegaSync *sync)
{
    if (sync)
    {
        mSync.reset(sync->copy());

        assert(mBackupId == INVALID_HANDLE || mBackupId == sync->getBackupId());
        mBackupId = sync->getBackupId();
        mEnabled = sync->isEnabled();

        mActive = sync->isActive(); //override active with the actual value
    }
    else
    {
        assert("SyncSettings constructor with null sync");
        mSync.reset(new MegaSync()); // MegaSync getter return fair enough defaults
    }
}

QString SyncSetting::getLocalFolder() const
{
    auto toret = QString::fromUtf8(mSync->getLocalFolder());
#ifdef WIN32
    if (toret.startsWith(QString::fromAscii("\\\\?\\")))
    {
        toret = toret.mid(4);
    }
#endif
    return toret;
}

long long SyncSetting::getLocalFingerprint()  const
{
    return mSync->getLocalFingerprint();
}

QString SyncSetting::getMegaFolder()  const
{
    if (mMegaFolder.isEmpty())
    {
        auto folder = mSync->getLastKnownMegaFolder();
        return folder ? QString::fromUtf8(folder) : QString();
    }

    return mMegaFolder;
}

MegaHandle SyncSetting::getMegaHandle()  const
{
    return mSync->getMegaHandle();
}

bool SyncSetting::isEnabled()  const
{
    return mEnabled;
}

bool SyncSetting::isActive()  const
{
    return mActive;
}

bool SyncSetting::isTemporaryDisabled()  const
{
    return mSync->isTemporaryDisabled();
}

int SyncSetting::getError() const
{
    return mSync->getError();
}

MegaHandle SyncSetting::backupId() const
{
    return mBackupId;
}

void SyncSetting::setBackupId(MegaHandle backupId)
{
    mBackupId = backupId;
}

MegaSync::SyncType SyncSetting::getType()
{
    return static_cast<MegaSync::SyncType>(mSync->getType());
}
