#pragma once

#include <iostream>
#include <QString>
#include <QLocale>
#include <QStringList>
#include <QMutex>
#include <QDataStream>

#include <assert.h>
#include <memory>

//TODO: reduce includes

#include "megaapi.h"

class SyncSetting
{
private:
    std::unique_ptr<mega::MegaSync> mSync; //shall not need to be persisted
    int mTag = 0;
    QString mName;
    QString mMegaFolder;
    bool mEnabled = false; //we need to hold this, for transitioning from old sync data, instead of simply forwarding to mSync->isEnabled

public:
    SyncSetting();
    SyncSetting(QString initializer);
    ~SyncSetting();
    SyncSetting(const SyncSetting& a);
    SyncSetting(SyncSetting&& a) = default;
    SyncSetting& operator=(const SyncSetting& a);
    SyncSetting& operator=(SyncSetting&& a) = default;


    SyncSetting(mega::MegaSync *sync, const char *value = nullptr);
    int tag() const;
    void setTag(int tag);
    QString name() const;
    void setName(const QString &name);

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

    void setMegaFolder(const char *value);

    mega::MegaSync* getSync() const;

    QString toString();

};

Q_DECLARE_METATYPE(SyncSetting);

struct SyncData
{
    SyncData(QString name, QString localFolder, long long  megaHandle,
                long long localfp, bool enabled, int pos, QString syncID);
    QString mName;
    QString mLocalFolder;
    long long mMegaHandle;
    long long mLocalfp;
    bool mEnabled;
    int mPos;
    QString mSyncID;
};
