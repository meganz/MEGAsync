#include "UpsellModel.h"

#include "QmlManager.h"
#include "UpsellController.h"
#include "UpsellPlans.h"

UpsellModel::UpsellModel(std::shared_ptr<UpsellController> controller, QObject* parent):
    QAbstractListModel(parent),
    mController(controller)
{
    connect(controller.get(),
            &UpsellController::beginInsertRows,
            this,
            [this](int first, int last)
            {
                beginInsertRows(QModelIndex(), first, last);
            });

    connect(controller.get(),
            &UpsellController::endInsertRows,
            this,
            [this]()
            {
                endInsertRows();
            });

    connect(controller.get(),
            &UpsellController::dataChanged,
            this,
            [this](int rowStart, int rowFinal, QVector<int> roles)
            {
                emit dataChanged(QModelIndex(index(rowStart, 0)),
                                 QModelIndex(index(rowFinal, 0)),
                                 roles);
            });

    QmlManager::instance()->setRootContextProperty(this);
}

QHash<int, QByteArray> UpsellModel::roleNames() const
{
    return UpsellPlans::Data::roleNames();
}

int UpsellModel::rowCount(const QModelIndex& parent) const
{
    // When implementing a table based model, rowCount() should return 0 when the parent is valid.
    return parent.isValid() ? 0 : mController->getPlans()->size();
}

bool UpsellModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return mController->setData(index.row(), value, role);
}

QVariant UpsellModel::data(const QModelIndex& index, int role) const
{
    return mController->data(index.row(), role);
}
