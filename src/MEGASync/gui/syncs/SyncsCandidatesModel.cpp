#include "SyncsCandidatesModel.h"

#include <QQmlContext>

#include <vector>

SyncsCandidatesModel::SyncsCandidatesModel(QObject* parent) {}

QHash<int, QByteArray> SyncsCandidatesModel::roleNames() const
{
    static QHash<int, QByteArray> roles{{SyncsCandidadteModelRole::LOCAL_FOLDER, "localFolder"},
                                        {SyncsCandidadteModelRole::MEGA_FOLDER, "megaFolder"}};

    return roles;
}

bool SyncsCandidatesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    auto& syncCandidate = mSyncCandidates[static_cast<size_t>(index.row())];

    switch (role)
    {
        case SyncsCandidadteModelRole::LOCAL_FOLDER:
            syncCandidate.first = value.toString().toStdString();
            break;

        case SyncsCandidadteModelRole::MEGA_FOLDER:
            syncCandidate.second = value.toString().toStdString();
            break;
    }

    return true;
}

QVariant SyncsCandidatesModel::data(const QModelIndex& index, int role) const
{
    QVariant returnValue;

    const auto row = index.row();
    if (row >= rowCount())
    {
        return returnValue;
    }

    auto& syncCandidate = mSyncCandidates[static_cast<size_t>(index.row())];

    switch (role)
    {
        case SyncsCandidadteModelRole::LOCAL_FOLDER:
        {
            returnValue = QString::fromUtf8(syncCandidate.first.c_str());
            break;
        }

        case SyncsCandidadteModelRole::MEGA_FOLDER:
        {
            returnValue = QString::fromUtf8(syncCandidate.second.c_str());
            break;
        }
    }

    return returnValue;
}

QModelIndex SyncsCandidatesModel::index(int row, int column, const QModelIndex& modelIndex) const
{
    return (row < rowCount(QModelIndex())) ? createIndex(row, column) : QModelIndex();
}

QModelIndex SyncsCandidatesModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int SyncsCandidatesModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

int SyncsCandidatesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(mSyncCandidates.size());
}

void SyncsCandidatesModel::add(const QString& localSyncFolder, const QString& megaSyncFolder)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    auto foundItem = exist(localSyncFolder, megaSyncFolder);
    if (!foundItem.has_value())
    {
        mSyncCandidates.emplace_back(localSyncFolder.toStdString(), megaSyncFolder.toStdString());
    }
    endInsertRows();
}

void SyncsCandidatesModel::reset()
{
    beginResetModel();
    mSyncCandidates.clear();
    endResetModel();
}

void SyncsCandidatesModel::remove(const QString& localSyncFolder, const QString& megaSyncFolder)
{
    auto foundItem = exist(localSyncFolder, megaSyncFolder);

    if (foundItem.has_value())
    {
        auto indexToRemove =
            static_cast<int>(std::distance(mSyncCandidates.begin(), foundItem.value()));
        beginRemoveRows(QModelIndex(), indexToRemove, indexToRemove);
        mSyncCandidates.erase(foundItem.value());
        endRemoveRows();
    }
}

void SyncsCandidatesModel::edit(const QString& originalLocalSyncFolder,
                                const QString& originalMegaSyncFolder,
                                const QString& localSyncFolder,
                                const QString& megaSyncFolder)
{
    auto foundItem = exist(localSyncFolder, megaSyncFolder);
    if (!foundItem.has_value())
    {
        auto foundOriginalItem = exist(originalLocalSyncFolder, originalMegaSyncFolder);
        if (foundOriginalItem.has_value())
        {
            beginResetModel();
            auto& syncCandidate = foundOriginalItem.value();
            syncCandidate->first = localSyncFolder.toStdString();
            syncCandidate->second = megaSyncFolder.toStdString();
            endResetModel();
        }
    }
}

bool SyncsCandidatesModel::exist(const QString& path,
                                 SyncsCandidatesModel::SyncsCandidadteModelRole syncsCandidateRole)
{
    auto itSyncFound =
        std::find_if(mSyncCandidates.begin(),
                     mSyncCandidates.end(),
                     [path, syncsCandidateRole](const auto& syncCandidate)
                     {
                         return (syncsCandidateRole == SyncsCandidadteModelRole::LOCAL_FOLDER &&
                                 syncCandidate.first == path.toStdString()) ||
                                (syncsCandidateRole == SyncsCandidadteModelRole::MEGA_FOLDER &&
                                 syncCandidate.second == path.toStdString());
                     });

    return (itSyncFound != mSyncCandidates.end());
}

std::optional<std::vector<SyncsCandidatesModel::SyncCandidate>::iterator>
    SyncsCandidatesModel::exist(const QString& localSyncFolder, const QString& megaSyncFolder)
{
    auto itSyncFound =
        std::find_if(mSyncCandidates.begin(),
                     mSyncCandidates.end(),
                     [localSyncFolder, megaSyncFolder](const auto& syncCandidate)
                     {
                         return syncCandidate.first == localSyncFolder.toStdString() &&
                                syncCandidate.second == megaSyncFolder.toStdString();
                     });

    if (itSyncFound != mSyncCandidates.end())
    {
        return itSyncFound;
    }

    return std::nullopt;
}
