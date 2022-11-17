#ifndef MEGAITEMMODEL_H
#define MEGAITEMMODEL_H

#include "MegaItem.h"
#include "Utilities.h"

#include <megaapi.h>

#include <QAbstractItemModel>
#include <QList>
#include <QIcon>
#include <QPointer>

#include <memory>

namespace UserAttributes{
class CameraUploadFolder;
class MyChatFilesFolder;
}

enum class MegaItemModelRoles
{
    DATE_ROLE = Qt::UserRole,
    IS_FILE_ROLE,
    STATUS_ROLE,
    last
};

enum class NodeRowDelegateRoles
{
    ENABLED_ROLE = toInt(MegaItemModelRoles::last),  //ALWAYS use last enum value from previous enum class for new enums
    INDENT_ROLE,
    last
};

class MegaItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    static const int ROW_HEIGHT;

    enum COLUMN{
      NODE = 0,
      STATUS,
      USER,
      DATE,
      last
    };

    explicit MegaItemModel(QObject *parent = 0);

    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;

    void setDisableFolders(bool option);
    void setSyncSetupMode(bool value);
    void showFiles(bool show);
    void addNode(std::unique_ptr<mega::MegaNode> node, const QModelIndex &parent);
    void removeNode(const QModelIndex &item);

    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex &index) const;
    QVariant getIcon(const QModelIndex &index, MegaItem* item) const;
    QVariant getText(const QModelIndex &index, MegaItem* item) const;

    virtual ~MegaItemModel();

private slots:
    void onItemInfoUpdated(int role);
    void onMyBackupsFolderHandleSet(mega::MegaHandle h);

protected:
    QList<MegaItem *> mRootItems;
    int mRequiredRights;
    bool mDisplayFiles;
    bool mSyncSetupMode;

private:
    int insertPosition(const std::unique_ptr<mega::MegaNode>& node);
    QModelIndex findItemByNodeHandle(const mega::MegaHandle &handle, const QModelIndex& parent);
    QIcon getFolderIcon(MegaItem* item) const;
    std::shared_ptr<const UserAttributes::CameraUploadFolder> mCameraFolderAttribute;
    std::shared_ptr<const UserAttributes::MyChatFilesFolder> mMyChatFilesFolderAttribute;
    mega::MegaApi* mMegaApi;
};

#endif // MEGAITEMMODEL_H
