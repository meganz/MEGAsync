#pragma once

#include <memory>

#include <QString>
#include <QDataStream>

#include "megaapi.h"

class SyncData;
class SyncSetting
{
private:
    std::unique_ptr<mega::MegaSync> mSync; //shall not need to be persisted
    int mTag = 0;
    QString mSyncID;

    bool mEnabled = false;
    bool mActive = false;

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
    int tag() const;
    void setTag(int tag);
    QString name() const;

    void setEnabled(bool value);

    void setSync(mega::MegaSync *sync);

    QString getLocalFolder() const;
    long long getLocalFingerprint() const;
    QString getMegaFolder() const;
    long long getMegaHandle() const;
    bool isEnabled() const; //enabled by user
    bool isActive() const; //not disabled by user nor failed (nor being removed)
    int getState() const;
    bool isTemporaryDisabled() const;
    int getError() const;

    mega::MegaSync* getSync() const;

    QString toString();

    QString getSyncID() const;
    void setSyncID(const QString &syncID);
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
