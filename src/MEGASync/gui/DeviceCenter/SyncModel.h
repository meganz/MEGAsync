#ifndef SYNC_MODEL_H
#define SYNC_MODEL_H

#include <QAbstractListModel>

class SyncModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum SyncModelRole
    {
        TYPE = Qt::UserRole + 1,
        NAME,
        SIZE,
        DATE_ADDED,
        DATE_MODIFIED
    };

    explicit SyncModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QString getName(int row) const;
    int getSize(int row) const;
    QString getType(int row) const;
    QDate getDateAdded(int row) const;
    QDate getDateModified(int row) const;
};

#endif // SYNC_MODEL_H
