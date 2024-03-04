#pragma once

#include <QString>
#include <QDataStream>

#include "megaapi.h"

#include <memory>

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

class SyncSettings
{
private:
    std::unique_ptr<mega::MegaSync> mSync; //shall not need to be persisted
    mega::MegaHandle mBackupId = ::mega::INVALID_HANDLE; //identifier given by the api
    QString mSyncID; //some id for platform specific settings

    QString mMegaFolder; //cached (in memory) value of the remote path

    static constexpr int CACHE_VERSION = 1;
public:

    SyncSettings();
    SyncSettings(const SyncData &osd, bool loadedFromPreviousSessions);
    SyncSettings(QString initializer);
    ~SyncSettings();
    SyncSettings(const SyncSettings& a);
    SyncSettings(SyncSettings&& a) = default;
    SyncSettings& operator=(const SyncSettings& a);
    SyncSettings& operator=(SyncSettings&& a) = default;

    SyncSettings(mega::MegaSync *sync);
    mega::MegaHandle backupId() const;
    void setBackupId(mega::MegaHandle backupId);
    // returns sync name verbatim or removing problematic chars (if removeUnsupportedChars = true)
    QString name(bool removeUnsupportedChars = false, bool normalizeDisplay = false) const;

    void setSync(mega::MegaSync *sync);

    QString getLocalFolder(bool normalizeDisplay = false) const;
    QString getMegaFolder() const;
    mega::MegaHandle getMegaHandle() const;
    int getError() const;
    int getWarning() const;
    bool isActive() const;

    int getRunState() const;

    mega::MegaSync* getSync() const;

    QString toString();

    QString getSyncID() const;
    void setSyncID(const QString &syncID);
    void setMegaFolder(const QString &megaFolder);

    mega::MegaSync::SyncType getType();
};

Q_DECLARE_METATYPE(SyncSettings)
Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<SyncSettings>)
