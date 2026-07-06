#include "SyncSettingsModelBase.h"

#include "MegaApplication.h"
#include "SyncInfo.h"
#include "Utilities.h"

#include <QCoreApplication>

SyncSettingsModelBase::SyncSettingsModelBase(mega::MegaSync::SyncType type, QObject* parent):
    QAbstractListModel(parent),
    mSyncInfo(SyncInfo::instance()),
    mType(type)
{
    mQCollator.setNumericMode(true);
    mQCollator.setCaseSensitivity(Qt::CaseInsensitive);

    connect(mSyncInfo, &SyncInfo::syncStateChanged, this, &SyncSettingsModelBase::insertItem);
    connect(mSyncInfo, &SyncInfo::syncStatsUpdated, this, &SyncSettingsModelBase::updateStats);
    connect(mSyncInfo, &SyncInfo::syncRemoved, this, &SyncSettingsModelBase::removeItem);
    connect(MegaSyncApp,
            &MegaApplication::languageChanged,
            this,
            &SyncSettingsModelBase::onLanguageChanged);

    mList = mSyncInfo->getSyncSettingsByType(mType);
    sortByName(true);
}

void SyncSettingsModelBase::onSyncRemoveBegins(mega::MegaHandle backupId)
{
    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&backupId](auto& sync)
                                {
                                    return (sync->backupId() == backupId);
                                });
    {
        QMutexLocker locker(&mRemoveSyncsMutex);
        mRemovingSyncs.append(backupId);
    }

    if (foundIt != mList.cend())
    {
        sendDataChanged(static_cast<int>(std::distance(mList.cbegin(), foundIt)));
    }
}

void SyncSettingsModelBase::onLanguageChanged()
{
    if (rowCount() == 0)
        return;

    emit dataChanged(index(0), index(rowCount() - 1), {Role::ErrorMessage});
}

void SyncSettingsModelBase::onSyncRemoveEnds(mega::MegaHandle backupId)
{
    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&backupId](auto& sync)
                                {
                                    return (sync->backupId() == backupId);
                                });
    {
        QMutexLocker locker(&mRemoveSyncsMutex);
        mRemovingSyncs.removeAll(backupId);
    }

    if (foundIt != mList.cend())
    {
        sendDataChanged(static_cast<int>(std::distance(mList.cbegin(), foundIt)));
    }
}

void SyncSettingsModelBase::sortByName(bool ascending)
{
    mSortAscending = ascending;

    emit layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);
    std::sort(mList.begin(),
              mList.end(),
              [this, ascending](const auto& sync1, const auto& sync2)
              {
                  const auto result =
                      mQCollator.compare(sync1->name(false, true), sync2->name(false, true));
                  return ascending ? result <= 0 : result > 0;
              });
    emit layoutChanged({}, QAbstractItemModel::VerticalSortHint);
}

void SyncSettingsModelBase::sortByStatus(bool ascending)
{
    mSortAscending = ascending;

    emit layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);
    std::sort(mList.begin(),
              mList.end(),
              [this, ascending](const auto& sync1, const auto& sync2)
              {
                  return ascending ? getState(sync1) <= getState(sync2) :
                                     getState(sync1) > getState(sync2);
              });
    emit layoutChanged({}, QAbstractItemModel::VerticalSortHint);
}

void SyncSettingsModelBase::insertItem(std::shared_ptr<SyncSettings> sync)
{
    if (sync->getType() != mType)
    {
        return;
    }

    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&sync](auto& listSync)
                                {
                                    return (sync == listSync);
                                });

    if (foundIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundIt);
        sendDataChanged(static_cast<int>(row));
    }
    else
    {
        beginInsertRows(QModelIndex(),
                        static_cast<int>(mList.size()),
                        static_cast<int>(mList.size()));
        mList.append(sync);
        endInsertRows();
    }
}

void SyncSettingsModelBase::sendDataChanged(int row)
{
    const auto modelIndex = index(row, 0, QModelIndex());

    emit dataChanged(modelIndex,
                     modelIndex,
                     QVector<int>() << Role::FolderRole << Role::StatusRole << Role::StatusId
                                    << Role::ErrorMessage << Role::NameRole << Role::ErrorId);
}

void SyncSettingsModelBase::updateStats(std::shared_ptr<::mega::MegaSyncStats> stats)
{
    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&stats](auto& listSync)
                                {
                                    return (stats->getBackupId() == listSync->backupId());
                                });

    if (foundIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundIt);
        sendDataChanged(static_cast<int>(row));
    }
}

void SyncSettingsModelBase::removeItem(std::shared_ptr<SyncSettings> sync)
{
    auto foundIt = std::find_if(mList.cbegin(),
                                mList.cend(),
                                [&sync](auto& listSync)
                                {
                                    return (sync == listSync);
                                });

    if (foundIt != mList.cend())
    {
        auto row = std::distance(mList.cbegin(), foundIt);
        auto pos = static_cast<int>(row);

        beginRemoveRows(QModelIndex(), pos, pos);
        mList.removeOne((*foundIt));
        endRemoveRows();
    }
}

QHash<int, QByteArray> SyncSettingsModelBase::roleNames() const
{
    return {
        {NameRole, "name"},
        {FolderRole, "folder"},
        {StatusRole, "status"},
        {ErrorMessage, "error"},
        {ErrorId, "error_id"},
    };
}

int SyncSettingsModelBase::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return static_cast<int>(mList.size());
}

QVariant SyncSettingsModelBase::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto row = index.row();

    if (row < 0 || row >= rowCount())
    {
        return {};
    }

    const auto& sync = mList[row];

    switch (role)
    {
        case FolderRole:
            return sync->getLocalFolder();

        case NameRole:
            return sync->name(false, true);

        case StatusRole:
            return getState(sync);

        case ErrorId:
            return sync->getError();

        case ErrorMessage:
            return getErrorMessage(sync);

        default:
            return {};
    }
}

QString SyncSettingsModelBase::getErrorMessage(std::shared_ptr<SyncSettings> sync) const
{
    std::unique_ptr<const char[]> syncErrorText(
        mega::MegaSync::getMegaSyncErrorCode(sync->getError()));

    return QCoreApplication::translate("MegaSyncError", syncErrorText.get());
}

std::shared_ptr<SyncSettings> SyncSettingsModelBase::getSyncSetting(int index) const
{
    return mList.at(index);
}

SyncSettingsModelBase::State
    SyncSettingsModelBase::getState(std::shared_ptr<SyncSettings> sync) const
{
    {
        QMutexLocker locker(&mRemoveSyncsMutex);
        if (mRemovingSyncs.contains(sync->backupId()))
        {
            return State::REMOVING;
        }
    }

    switch (sync->getRunState())
    {
        case ::mega::MegaSync::RUNSTATE_PENDING:
        {
            return State::PENDING;
        }
        case ::mega::MegaSync::RUNSTATE_LOADING:
        {
            return State::LOADING;
        }
        case ::mega::MegaSync::RUNSTATE_SUSPENDED:
        {
            if (sync->getError())
            {
                return State::FAIL;
            }
            else
            {
                return State::SUSPENDED;
            }
        }
        case ::mega::MegaSync::RUNSTATE_DISABLED:
        {
            if (sync->getError())
            {
                return State::FAIL;
            }
            break;
        }
        case ::mega::MegaSync::RUNSTATE_RUNNING:
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                auto stats = it->second;
                if (stats->isScanning())
                {
                    return State::SCANNING;
                }
                else if (stats->isSyncing())
                {
                    return State::ACTIVE;
                }
            }
            break;
        }
    }

    return State::IDLE;
}
