#ifndef BACKUPSMODEL_H
#define BACKUPSMODEL_H

#include "SyncController.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QTimer>

class BackupCandidatesController;

class BackupCandidatesModel: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit BackupCandidatesModel(std::shared_ptr<BackupCandidatesController> controller,
                                   QObject* parent = nullptr);
    ~BackupCandidatesModel();

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& index, int role) const override;
    void reset();

private:
    friend class BackupCandidatesController;
    std::shared_ptr<BackupCandidatesController> mBackupCandidatesController;
};

class BackupCandidatesProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool selectedFilterEnabled READ selectedFilterEnabled WRITE setSelectedFilterEnabled
                   NOTIFY selectedFilterEnabledChanged)

public:
    explicit BackupCandidatesProxyModel(std::shared_ptr<BackupCandidatesController> controller,
                                        QObject* parent = nullptr);
    BackupCandidatesProxyModel() = default;

    bool selectedFilterEnabled() const;
    void setSelectedFilterEnabled(bool enabled);

signals:
    void selectedFilterEnabledChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
    bool mSelectedFilterEnabled;
    std::shared_ptr<BackupCandidatesModel> mBackupsModel;
};

#endif // BACKUPSMODEL_H
