#ifndef SYNC_MODEL_H
#define SYNC_MODEL_H

#include "megaapi.h"
#include "QmlSyncData.h"

#include <optional>
#include <QAbstractListModel>

class SyncModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum SyncModelRole
    {
        TYPE = Qt::UserRole + 1,
        NAME,
        SIZE,
        DATE_ADDED,
        DATE_MODIFIED,
        STATUS
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

public:
    QString getName(int row) const;
    QString getSize(int row) const;
    QmlSyncType::Type getType(int row) const;
    QDate getDateAdded(int row) const;
    QDate getDateModified(int row) const;
    SyncStatus::Value getStatus(int row) const;
    std::optional<int> findRowByHandle(mega::MegaHandle handle) const;

    QList<QmlSyncData> mSyncObjects;
};

#endif // SYNC_MODEL_H
