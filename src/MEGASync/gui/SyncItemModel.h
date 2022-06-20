#ifndef SYNCITEMMODEL_H
#define SYNCITEMMODEL_H


#include "../model/SyncModel.h"
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
    QList<std::shared_ptr<SyncSetting>> getList() const;
    void setList(QList<std::shared_ptr<SyncSetting>> list);
    mega::MegaSync::SyncType getMode();
    void setMode(mega::MegaSync::SyncType syncType);

signals:
    void enableSync(std::shared_ptr<SyncSetting> syncSetting);
    void disableSync(std::shared_ptr<SyncSetting> syncSetting);
    void syncUpdateFinished(std::shared_ptr<SyncSetting> syncSetting);

private slots:
    //void resetModel();
    void insertSync(std::shared_ptr<SyncSetting> sync);
    void removeSync(std::shared_ptr<SyncSetting> sync);

private:
    SyncModel* mSyncModel;
    QList<std::shared_ptr<SyncSetting>> mList;
    mega::MegaSync::SyncType mSyncType;
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



