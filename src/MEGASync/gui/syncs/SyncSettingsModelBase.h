#ifndef SYNC_SETTINGS_MODEL_BASE_H
#define SYNC_SETTINGS_MODEL_BASE_H

#include "megaapi.h"

#include <QAbstractListModel>
#include <QCollator>

#include <memory>
#include <qmutex.h>

class SyncInfo;
class SyncSettings;

/**
 * @brief Common list model shared by the Syncs and Backups settings tabs.
 *
 * Both tabs display the same kind of rows (name, folder, status, error) backed by
 * SyncInfo; they only differ in which SyncType they show. Subclasses pass that type
 * to the constructor and are otherwise empty, existing solely so each tab registers
 * as its own QML type.
 */
class SyncSettingsModelBase: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SyncSettingsModelBase(mega::MegaSync::SyncType type, QObject* parent = nullptr);
    ~SyncSettingsModelBase() override = default;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    std::shared_ptr<SyncSettings> getSyncSetting(int index) const;
    void sortByName(bool ascending = true);
    void sortByStatus(bool ascending = true);

    // Shared status enum. The tail values (ACTIVE/IDLE) map to syncing/synced on the
    // Syncs tab and backing-up/backed-up on the Backups tab; the displayed text is
    // resolved per tab in QML via SettingsStrings, the integer values are identical.
    enum State
    {
        PENDING,
        LOADING,
        SUSPENDED,
        FAIL,
        SCANNING,
        ACTIVE,
        IDLE,
        REMOVING
    };

    Q_ENUM(State)

protected:
    enum Role
    {
        NameRole = Qt::UserRole + 1,
        FolderRole,
        StatusRole,
        StatusId,
        ErrorMessage,
        ErrorId
    };

    void onSyncRemoveBegins(mega::MegaHandle);
    void onSyncRemoveEnds(mega::MegaHandle);

private slots:
    void insertItem(std::shared_ptr<SyncSettings> sync);
    void updateStats(std::shared_ptr<mega::MegaSyncStats> stats);
    void removeItem(std::shared_ptr<SyncSettings> sync);

private:
    State getState(std::shared_ptr<SyncSettings> sync) const;
    void sendDataChanged(int row);
    QString getErrorMessage(std::shared_ptr<SyncSettings> sync) const;
    void onLanguageChanged();

    SyncInfo* mSyncInfo;
    mega::MegaSync::SyncType mType;
    QList<std::shared_ptr<SyncSettings>> mList;
    bool mSortAscending = true;
    QCollator mQCollator;
    QList<mega::MegaHandle> mRemovingSyncs;
    mutable QMutex mRemoveSyncsMutex;
};

#endif // SYNC_SETTINGS_MODEL_BASE_H
