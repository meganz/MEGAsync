#ifndef SYNCS_DATA_H
#define SYNCS_DATA_H

#include "Syncs.h"

#include <QObject>

class SyncsData: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString defaultMegaFolder READ getDefaultMegaFolder CONSTANT FINAL)
    Q_PROPERTY(QString defaultMegaPath READ getDefaultMegaPath CONSTANT FINAL)
    Q_PROPERTY(Syncs::SyncStatusCode syncStatus READ getSyncStatus NOTIFY syncStatusChanged)
    Q_PROPERTY(QString localError READ getLocalError NOTIFY localErrorChanged)
    Q_PROPERTY(QString remoteError READ getRemoteError NOTIFY remoteErrorChanged)

public:
    SyncsData(const Syncs* const syncs);
    virtual ~SyncsData() = default;

signals:
    void syncStatusChanged();
    void localErrorChanged();
    void remoteErrorChanged();

private:
    QString getDefaultMegaFolder() const;
    QString getDefaultMegaPath() const;
    Syncs::SyncStatusCode getSyncStatus() const;
    QString getLocalError() const;
    QString getRemoteError() const;

    const Syncs* mSyncs = nullptr;
};

#endif
