#include "UpdatesModel.h"

#include "QmlManager.h"

UpdatesModel::UpdatesModel(std::shared_ptr<WhatsNewController> controller, QObject* parent):
    QAbstractListModel(parent),
    mUpdatesController(controller)
{
    QmlManager::instance()->setRootContextProperty(this);
}

QHash<int, QByteArray> UpdatesModel::roleNames() const
{
    if (!mUpdatesController)
    {
        return {};
    }
    return mUpdatesController->roleNames();
}

int UpdatesModel::rowCount(const QModelIndex& parent) const
{
    if (!mUpdatesController)
    {
        return {};
    }
    return mUpdatesController->rowCount();
}

QVariant UpdatesModel::data(const QModelIndex& index, int role) const
{
    if (!mUpdatesController)
    {
        return {};
    }
    return mUpdatesController->data(index, role);
}
