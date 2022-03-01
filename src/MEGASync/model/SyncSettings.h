#pragma once

#include <memory>

#include <QString>
#include <QDataStream>

#include "megaapi.h"

struct SyncData;
class SyncSetting
{
private:
    std::unique_ptr<mega::MegaSync> mSync; //shall not need to be persisted
    mega::MegaHandle mBackupId = ::mega::INVALID_HANDLE; //identifier given by the api
    QString mSyncID; //some id for platform specific settings

    bool mEnabled = false;
    bool mActive = false;

    QString mMegaFolder; //cached (in memory) value of the remote path

    static constexpr int CACHE_VERSION = 1;
public:
    SyncSetting();
    SyncSetting(const SyncData &osd, bool loadedFromPreviousSessions);
    SyncSetting(QString initializer);
    ~SyncSetting();
    SyncSetting(const SyncSetting& a);
    SyncSetting(SyncSetting&& a) = default;
    SyncSetting& operator=(const SyncSetting& a);
    SyncSetting& operator=(SyncSetting&& a) = default;

    SyncSetting(mega::MegaSync *sync);
    mega::MegaHandle backupId() const;
    void setBackupId(mega::MegaHandle backupId);
    // returns sync name verbatim or removing problematic chars (if removeUnsupportedChars = true)
    QString name(bool removeUnsupportedChars = false) const;

    void setEnabled(bool value);

    void setSync(mega::MegaSync *sync);

    QString getLocalFolder() const;
    long long getLocalFingerprint() const;
    QString getMegaFolder() const;
    mega::MegaHandle getMegaHandle() const;
    int getError() const;

    QString getRunStateAsString() const;

    mega::MegaSync* getSync() const;

    QString toString();

    QString getSyncID() const;
    void setSyncID(const QString &syncID);
    void setMegaFolder(const QString &megaFolder);
};

Q_DECLARE_METATYPE(SyncSetting);

struct SyncData
{
    SyncData(QString name, QString localFolder, long long megaHandle, QString megaFolder,
                long long localfp, bool enabled, bool tempDisabled, int pos, QString syncID);
    QString mName;
    QString mLocalFolder;
    long long mMegaHandle;
    QString mMegaFolder;
    long long mLocalfp;
    bool mEnabled;
    bool mTemporarilyDisabled;
    int mPos;
    QString mSyncID;
};
