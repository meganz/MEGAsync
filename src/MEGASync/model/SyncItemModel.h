#ifndef SYNCITEMMODEL_H
#define SYNCITEMMODEL_H

#include "SyncModel.h"
#include "SyncController.h"

#include <QSortFilterProxyModel>
#include <QAbstractItemModel>

#include <QCollator>

class SyncItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    typedef enum
    {
        ENABLED = 0,
        LNAME,
        RNAME,
        MENU
    } Column;
    const unsigned int kColumns = 4;

    static const int ICON_SIZE;
    static const int WARNING_ICON_SIZE;

    explicit SyncItemModel(QObject *parent = nullptr);

    // Header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual void fillData();

protected:
    QList<std::shared_ptr<SyncSettings>> getList() const;
    void setList(QList<std::shared_ptr<SyncSettings>> list);
    mega::MegaSync::SyncType getMode();
    void setMode(mega::MegaSync::SyncType syncType);

signals:
    void enableSync(std::shared_ptr<SyncSettings> syncSetting);
    void disableSync(std::shared_ptr<SyncSettings> syncSetting);
    void syncUpdateFinished(std::shared_ptr<SyncSettings> syncSetting);

private slots:
    //void resetModel();
    void insertSync(std::shared_ptr<SyncSettings> sync);
    void removeSync(std::shared_ptr<SyncSettings> sync);

private:
    SyncModel* mSyncModel;
    QList<std::shared_ptr<SyncSettings>> mList;
    mega::MegaSync::SyncType mSyncType;

    virtual void sendDataChanged(int row);
};


class SyncItemSortModel : public QSortFilterProxyModel
{
public:
    explicit SyncItemSortModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    QCollator mQCollator;
};

#endif // SYNCITEMMODEL_H



