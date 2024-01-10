#ifndef SYNCITEMMODEL_H
#define SYNCITEMMODEL_H

#include "syncs/control/SyncController.h"

#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <QCollator>

class SyncInfo;
class SyncItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column
    {
        ENABLED = 0,
        LNAME,
        STATE,
        FILES,
        FOLDERS,
        DOWNLOADS,
        UPLOADS,
        MENU
    };
    const unsigned int kColumns = 8;

    static const int ICON_SIZE;
    static const int STATES_ICON_SIZE;

    static const int ErrorTooltipRole;

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

    std::shared_ptr<SyncSettings> getSyncSettings(const QModelIndex &index) const;

signals:
    void signalSyncCheckboxOn(std::shared_ptr<SyncSettings> syncSetting);
    void signalSyncCheckboxOff(std::shared_ptr<SyncSettings> syncSetting);
    void syncUpdateFinished(std::shared_ptr<SyncSettings> syncSetting);

protected:
    QList<std::shared_ptr<SyncSettings>> getList() const;
    void setList(QList<std::shared_ptr<SyncSettings>> list);
    mega::MegaSync::SyncType getMode();
    void setMode(mega::MegaSync::SyncType syncType);

    SyncInfo* mSyncInfo;

private slots:
    //void resetModel();
    void insertSync(std::shared_ptr<SyncSettings> sync);
    void updateSyncStats(std::shared_ptr<::mega::MegaSyncStats> stats);
    void removeSync(std::shared_ptr<SyncSettings> sync);

private:
    QList<std::shared_ptr<SyncSettings>> mList;
    mega::MegaSync::SyncType mSyncType;

    virtual void sendDataChanged(int row);
    QVariant getColumnStats(int role, mega::MegaHandle backupId, std::function<int (const::mega::MegaSyncStats &)> statsGetter) const;
    QIcon getStateIcon(const std::shared_ptr<SyncSettings>& sync) const;
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



