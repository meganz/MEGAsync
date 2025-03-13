#ifndef SYNCS_DATA_H
#define SYNCS_DATA_H

#include "SyncsUtils.h"

#include <QObject>

class Syncs;
class SyncsData: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString defaultMegaFolder READ getDefaultMegaFolder CONSTANT FINAL)
    Q_PROPERTY(QString defaultMegaPath READ getDefaultMegaPath CONSTANT FINAL)
    Q_PROPERTY(SyncsUtils::SyncStatusCode syncStatus READ getSyncStatus NOTIFY syncStatusChanged)
    Q_PROPERTY(QString localError READ getLocalError NOTIFY localErrorChanged)
    Q_PROPERTY(QString remoteError READ getRemoteError NOTIFY remoteErrorChanged)

public:
    SyncsData(const Syncs* const syncs);
    virtual ~SyncsData() = default;
    static QString getDefaultMegaFolder();
    static QString getDefaultMegaPath();

signals:
    void syncStatusChanged();
    void localErrorChanged();
    void remoteErrorChanged();

private:
    SyncsUtils::SyncStatusCode getSyncStatus() const;
    QString getLocalError() const;
    QString getRemoteError() const;

    const Syncs* mSyncs = nullptr;
};

#endif
