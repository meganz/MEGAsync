
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

SyncSetting::SyncSetting(const SyncSetting& a) :
    mSync(a.getSync()->copy()), mTag(a.tag()), mName(a.name()), mMegaFolder(a.getMegaFolder())
{
}

SyncSetting& SyncSetting::operator=(const SyncSetting& a)
{
    mSync.reset(a.getSync()->copy());
    mTag = a.tag();
    mName = a.name();
    mMegaFolder = a.getMegaFolder();
    return *this;
}

SyncSetting::SyncSetting()
{
    mSync.reset(new MegaSync()); // MegaSync getters return fair enough defaults
}

SyncSetting::SyncSetting(MegaSync *sync, const char *remotePath)
{
    setSync(sync);
    setMegaFolder(remotePath);
}

QString SyncSetting::name() const
{
    return mName.size()? mName : getLocalFolder();
}

void SyncSetting::setName(const QString &name)
{
    mName = name;
}

MegaSync * SyncSetting::getSync() const
{
    return mSync.get();
}

SyncSetting::SyncSetting(QString initializer)
{
    QStringList parts = initializer.split(QString::fromUtf8("0x1E"));
    int i = 0;
    if (i<parts.size()) { mTag = parts.at(i++).toInt(); }
    if (i<parts.size()) { mName = parts.at(i++); }

    mSync.reset(new MegaSync()); // MegaSync getters return fair enough defaults
}

QString SyncSetting::toString()
{
    QStringList toret;
    toret.append(QString::number(mTag));
    toret.append(mName);

    return toret.join(QString::fromUtf8("0x1E"));
}

void SyncSetting::setSync(MegaSync *sync)
{
    if (sync)
    {
        if (!mName.size())
        {
            mName = QFileInfo(QString::fromUtf8(sync->getLocalFolder())).fileName();
        }
        mSync.reset(sync->copy());

        assert(mTag == 0 || mTag == sync->getTag());
        mTag = sync->getTag();
    }
    else
    {
        assert("SyncSettings constructor with null sync");
        mSync.reset(new MegaSync()); // MegaSync getter return fair enough defaults
    }
}

QString SyncSetting::getLocalFolder() const
{
    return QString::fromUtf8(mSync->getLocalFolder());
}

long long SyncSetting::getLocalFingerprint()  const
{
    return mSync->getLocalFingerprint();
}

QString SyncSetting::getMegaFolder()  const
{
    //TODO: use remote path (stored somewhere, probably within SyncConfig)
    return mMegaFolder;
}

long long SyncSetting::getMegaHandle()  const
{
    return mSync->getMegaHandle();
}

bool SyncSetting::isEnabled()  const
{
    return mSync->isEnabled();
}

bool SyncSetting::isTemporaryDisabled()  const
{
    return mSync->isTemporaryDisabled();
}

int SyncSetting::getError() const
{
    return mSync->getError();
}

void SyncSetting::setMegaFolder(const char *value)
{
    mMegaFolder = QString::fromUtf8(value);
}

int SyncSetting::tag() const
{
    return mTag;
}

void SyncSetting::setTag(int tag)
{
    mTag = tag;
}









