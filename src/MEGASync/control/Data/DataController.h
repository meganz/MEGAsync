#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <QObject>

class DataController: public QObject
{
    Q_OBJECT

public:
    DataController() = default;

    virtual bool setData(int row, const QVariant& value, int role) = 0;
    virtual QVariant data(int row, int role) const = 0;

    virtual int size() const = 0;

protected:
    void updateModel(int row, QVector<int> roles)
    {
        emit dataChanged(row, roles);
    }

signals:
    // Add row
    void beginInsertRows(int first, int last);
    void endInsertRows();

    // Remove row
    void beginRemoveRows(int first, int last);
    void endRemoveRows();

    // Update row
    void dataChanged(int row, QVector<int> rol);
};

#endif // DATACONTROLLER_H
