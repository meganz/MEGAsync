
#include "SyncSettings.h"
#include "platform/Platform.h"

#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif


SyncSettings::~SyncSettings()
{
}

SyncSettings::SyncSettings(const SyncSettings& a) :
    mSync(a.getSync()->copy()), mBackupId(a.backupId()),
    mSyncID(a.getSyncID()), mMegaFolder(a.mMegaFolder)
{
}

SyncSettings& SyncSettings::operator=(const SyncSettings& a)
{
    mSync.reset(a.getSync()->copy());
    mBackupId = a.backupId();
    mSyncID = a.getSyncID();
    mMegaFolder = a.mMegaFolder;
    return *this;
}

QString SyncSettings::getSyncID() const
{
    return mSyncID;
}

void SyncSettings::setSyncID(const QString &syncID)
{
    mSyncID = syncID;
}

void SyncSettings::setMegaFolder(const QString &megaFolder)
{
    mMegaFolder = megaFolder;
}

SyncSettings::SyncSettings()
{
    mSync.reset(new MegaSync()); // MegaSync getters return fair enough defaults
}

SyncSettings::SyncSettings(MegaSync *sync)
{
    setSync(sync);
}

QString SyncSettings::name(bool removeUnsupportedChars) const
{
    //Provide name removing ':' to avoid possible issues during communications with shell extension
    return removeUnsupportedChars ? QString::fromUtf8(mSync->getName()).remove(QChar::fromAscii(':'))
                                  : QString::fromUtf8(mSync->getName());
}

void SyncSettings::setEnabled(bool value)
{
    mEnabled = value;
}

MegaSync * SyncSettings::getSync() const
{
    return mSync.get();
}

SyncSettings::SyncSettings(QString initializer)
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

SyncSettings::SyncSettings(const SyncData &osd, bool/* loadedFromPreviousSessions*/)
{
    mSyncID = osd.mSyncID;
    mEnabled = osd.mEnabled;

    // Although mActive should be false for loadedFromPreviousSessions, since MEGAsync versions with old cache
    // did not deActivate syncs when logging out, we dont need to consider this
    // keeping the parameter and the code in case we consider fixing that. Uncoment the /**/ in that case.
    mActive = /*!loadedFromPreviousSessions && */osd.mEnabled && !osd.mTemporarilyDisabled;

}

QString SyncSettings::toString()
{
    QStringList toret;
    toret.append(QString::number(CACHE_VERSION));
    toret.append(QString::number(mBackupId));
    toret.append(mSyncID);

    return toret.join(QString::fromUtf8("0x1E"));
}

void SyncSettings::setSync(MegaSync *sync)
{
    if (sync)
    {
        mSync.reset(sync->copy());

        assert(mBackupId == INVALID_HANDLE || mBackupId == sync->getBackupId());
        mBackupId = sync->getBackupId();
    }
    else
    {
        assert("SyncSettings constructor with null sync");
        mSync.reset(new MegaSync()); // MegaSync getter return fair enough defaults
    }
}

QString SyncSettings::getLocalFolder() const
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

long long SyncSettings::getLocalFingerprint()  const
{
    return mSync->getLocalFingerprint();
}

QString SyncSettings::getMegaFolder()  const
{
    if (mMegaFolder.isEmpty())
    {
        auto folder = mSync->getLastKnownMegaFolder();
        return folder ? QString::fromUtf8(folder) : QString();
    }

    return mMegaFolder;
}

MegaHandle SyncSettings::getMegaHandle()  const
{
    return mSync->getMegaHandle();
}

int SyncSettings::getError() const
{
    return mSync->getError();
}

QString SyncSettings::getRunStateAsString() const
{
    switch (mSync->getRunState())
    {
    case MegaSync::RUNSTATE_PENDING: return QString::fromUtf8("Pending");
    case MegaSync::RUNSTATE_LOADING: return QString::fromUtf8("Loading");
    case MegaSync::RUNSTATE_RUNNING: return QString::fromUtf8("Running");
    case MegaSync::RUNSTATE_PAUSED: return QString::fromUtf8("Paused");
    case MegaSync::RUNSTATE_SUSPENDED: return QString::fromUtf8("Suspended");
    case MegaSync::RUNSTATE_DISABLED: return QString::fromUtf8("Disabled");
    }
    return QString::fromUtf8("<unknown state>");
}

int SyncSettings::getRunState() const
{
    assert(mSync);

    return mSync->getRunState();
}

MegaHandle SyncSettings::backupId() const
{
    return mBackupId;
}

void SyncSettings::setBackupId(MegaHandle backupId)
{
    mBackupId = backupId;
}

MegaSync::SyncType SyncSettings::getType()
{
    return static_cast<MegaSync::SyncType>(mSync->getType());
}
