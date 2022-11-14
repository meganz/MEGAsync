#ifndef BACKUPITEMMODEL_H
#define BACKUPITEMMODEL_H

#include "syncs/model/SyncItemModel.h"

namespace UserAttributes{
class DeviceName;
class MyBackupsHandle;
}

class BackupItemModel : public SyncItemModel
{
    Q_OBJECT

public:
    typedef enum
    {
        ENABLED = 0,
        LNAME,
        MENU
    } Column;
    const unsigned int kColumns = 3;

    explicit BackupItemModel(QObject *parent = nullptr);

    // Header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void fillData() override;

private:
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;

    void sendDataChanged(int pos) override;

};

#endif // BACKUPITEMMODEL_H
