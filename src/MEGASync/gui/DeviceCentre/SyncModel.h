#ifndef SYNC_MODEL_H
#define SYNC_MODEL_H

#include "megaapi.h"
#include "QmlSyncData.h"

#include <QAbstractListModel>

#include <memory>
#include <optional>

class SyncSettings;

class SyncModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum SyncModelRole
    {
        TYPE = Qt::UserRole + 1,
        NAME,
        SIZE,
        DATE_MODIFIED,
        STATUS,
        ERROR_MESSAGE,
        LOCAL_PATH,
        REMOTE_PATH
    };

    explicit SyncModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void add(const QmlSyncData& newSync);
    void addOrUpdate(const QmlSyncData& newSync);
    void remove(mega::MegaHandle handle);
    void clear();
    SyncStatus::Value computeDeviceStatus() const;
    qint64 computeTotalSize() const;

    void setStatus(mega::MegaHandle handle, const SyncStatus::Value status);
    bool hasUpdatingStatus() const;
    std::optional<mega::MegaHandle> getHandle(int row) const;
    std::optional<mega::MegaHandle> getSyncID(int row) const;
    QmlSyncType::Type getType(int row) const;
    QString getLocalFolder(int row) const;
    QString getRemoteFolder(int row) const;

private slots:
    void onSyncRootChanged(std::shared_ptr<SyncSettings> syncSettings);

private:
    QString getName(int row) const;
    QString getSize(int row) const;
    QDate getDateModified(int row) const;
    SyncStatus::Value getStatus(int row) const;
    QString getErrorMessage(int row) const;
    std::optional<int> findRowByHandle(mega::MegaHandle handle) const;

    QList<QmlSyncData> mSyncObjects;
};

#endif // SYNC_MODEL_H
