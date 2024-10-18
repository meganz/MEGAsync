#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractItemModel>

#include <memory>

template<class ControllerType>
class DataModel: public QAbstractItemModel
{
public:
    DataModel(std::shared_ptr<ControllerType> controller, QObject* parent):
        QAbstractItemModel(parent),
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

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override
    {
        return mController->setData(index.row(), value, role);
    }

    virtual QVariant data(const QModelIndex& index, int role) const override
    {
        return mController->data(index.row(), role);
    }

    /////////DEFAULT BEHAVIOUR: LIST MODEL. OVERRIDE IT TO CREATE MORE COMPLEX MODELS/////////77
    virtual QModelIndex index(int row,
                              int column,
                              const QModelIndex& = QModelIndex()) const override
    {
        return (row < rowCount(QModelIndex())) ? createIndex(row, column) : QModelIndex();
    }

    virtual QModelIndex parent(const QModelIndex&) const override
    {
        return QModelIndex();
    }

    virtual int columnCount(const QModelIndex& parent) const override
    {
        return 1;
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : mController->size();
    }

private:
    std::shared_ptr<ControllerType> mController;
};

#endif // DATAMODEL_H
