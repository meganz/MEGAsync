#ifndef BACKUPITEMMODEL_H
#define BACKUPITEMMODEL_H

#include <QAbstractItemModel>

#include "../model/SyncModel.h"

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(std::shared_ptr<SyncSetting>)

typedef enum
{
    ENABLED = 0,
    LNAME,
    MENU
} BackupItemColumn;

const unsigned int BACKUP_ITEM_COLUMNS = 3;

class BackupItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit BackupItemModel(QObject *parent = nullptr);

    // Header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    // Basic functionality
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Add data
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

signals:
    void enableSync(std::shared_ptr<SyncSetting> syncSetting);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting);

private slots:
    void resetModel();

private:
    SyncModel* mSyncModel;

    QList<std::shared_ptr<SyncSetting>> mList;
    QList<std::shared_ptr<SyncSetting>> mOrderedList;
    int mSortColumn;
    Qt::SortOrder mSortOrder;
};

#endif // BACKUPITEMMODEL_H
