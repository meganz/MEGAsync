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
            returnValue = QString::fromLatin1(syncCandidate.first.c_str());
            return returnValue;
        }

        case SyncsCandidadteModelRole::MEGA_FOLDER:
        {
            returnValue = QString::fromLatin1(syncCandidate.second.c_str());
            return returnValue;
        }

        default:
            return returnValue;
    }
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
    beginRemoveRows(QModelIndex(), 0, rowCount());
    mSyncCandidates.clear();
    endRemoveRows();
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
