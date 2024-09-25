#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractListModel>

template<class ControllerType>
class DataModel: public QAbstractListModel
{
public:
    DataModel(std::shared_ptr<ControllerType> controller, QObject* parent):
        QAbstractListModel(parent),
        mController(controller)
    {
        connect(controller.get(),
                &ControllerType::beginInsertRows,
                this,
                [this](int first, int last)
                {
                    beginInsertRows(QModelIndex(), first, last);
                });

        connect(controller.get(),
                &ControllerType::endInsertRows,
                this,
                [this]()
                {
                    endInsertRows();
                });

        connect(controller.get(),
                &ControllerType::beginRemoveRows,
                this,
                [this](int first, int last)
                {
                    beginRemoveRows(QModelIndex(), first, last);
                });

        connect(controller.get(),
                &ControllerType::endRemoveRows,
                this,
                [this]()
                {
                    endRemoveRows();
                });

        connect(controller.get(),
                &ControllerType::dataChanged,
                this,
                [this](int row, QVector<int> roles)
                {
                    QModelIndex updateIndex(index(row, 0));
                    emit dataChanged(updateIndex, updateIndex, roles);
                });
    }

    virtual QHash<int, QByteArray> roleNames() const override = 0;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        // When implementing a table based model, rowCount() should return 0 when the parent is
        // valid.
        return parent.isValid() ? 0 : mController->size();
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role) override
    {
        return mController->setData(index.row(), value, role);
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        return mController->data(index.row(), role);
    }

private:
    std::shared_ptr<ControllerType> mController;
};

#endif // DATAMODEL_H
