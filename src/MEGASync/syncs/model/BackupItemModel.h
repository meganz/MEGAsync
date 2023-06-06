#ifndef BACKUPITEMMODEL_H
#define BACKUPITEMMODEL_H

#include "syncs/model/SyncItemModel.h"

class BackupItemModel : public SyncItemModel
{
    Q_OBJECT

public:
    const unsigned int kColumns = 7;

    explicit BackupItemModel(QObject *parent = nullptr);

    // Header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void fillData() override;

private:
    void sendDataChanged(int pos) override;

};

#endif // BACKUPITEMMODEL_H
