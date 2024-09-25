#include "BackupCandidatesModel.h"

#include "BackupCandidatesController.h"
#include "megaapi.h"
#include "QmlManager.h"
#include "StandardIconProvider.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "Utilities.h"

#include <QQmlContext>

BackupCandidatesModel::BackupCandidatesModel(std::shared_ptr<BackupCandidatesController> controller,
                                             QObject* parent):
    DataModel<BackupCandidatesController>(controller, parent)
{}

BackupCandidatesModel::~BackupCandidatesModel()
{
    QmlManager::instance()->removeImageProvider(QLatin1String("standardicons"));
}

QHash<int, QByteArray> BackupCandidatesModel::roleNames() const
{
    return BackupCandidates::Data::roleNames();
}

// ************************************************************************************************
// * BackupCandidatesProxyModel
// ************************************************************************************************

BackupCandidatesProxyModel::BackupCandidatesProxyModel(
    std::shared_ptr<BackupCandidatesController> controller,
    QObject* parent):
    QSortFilterProxyModel(parent),
    mSelectedFilterEnabled(false)
{
    mBackupsModel = std::make_shared<BackupCandidatesModel>(controller);
    setSourceModel(mBackupsModel.get());
    setDynamicSortFilter(true);
    QmlManager::instance()->setRootContextProperty(this);
}

bool BackupCandidatesProxyModel::selectedFilterEnabled() const
{
    return mSelectedFilterEnabled;
}

void BackupCandidatesProxyModel::setSelectedFilterEnabled(bool enabled)
{
    if (mSelectedFilterEnabled == enabled)
    {
        return;
    }

    mSelectedFilterEnabled = enabled;
    emit selectedFilterEnabledChanged();

    invalidateFilter();
}

bool BackupCandidatesProxyModel::filterAcceptsRow(int sourceRow,
                                                  const QModelIndex& sourceParent) const
{
    if (!mSelectedFilterEnabled)
    {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return index.data(BackupsModel::BackupFolderRoles::SELECTED_ROLE).toBool();
}

QStringList BackupsProxyModel::getSelectedFolders() const
{
    QStringList selectedFolders;
    for (int row = 0; row < rowCount(); row++)
    {
        if (!index(row, 0).data(BackupsModel::BackupFolderRoles::DONE_ROLE).toBool())
        {
            QString folderPath(
                index(row, 0).data(BackupsModel::BackupFolderRoles::FOLDER_ROLE).toString());
            selectedFolders.append(folderPath);
        }
    }
    return selectedFolders;
}

void BackupsProxyModel::createBackups(SyncInfo::SyncOrigin origin)
{
    // if(!backupsModel()->checkDirectories())
    // {
    //     return;
    // }

    // // All expected errors have been handled
    // BackupsController::BackupInfoList candidateList;
    // for (int row = 0; row < rowCount(); row++)
    // {
    //     if(!index(row, 0).data(BackupsModel::BackupFolderRoles::DONE_ROLE).toBool())
    //     {
    //         BackupsController::BackupInfo candidate;
    //         candidate.first = index(row,
    //         0).data(BackupsModel::BackupFolderRoles::FOLDER_ROLE).toString(); candidate.second =
    //         index(row, 0).data(BackupsModel::BackupFolderRoles::NAME_ROLE).toString();
    //         candidateList.append(candidate);
    //     }
    // }
    // backupsModel()->backupsController()->addBackups(candidateList, origin);
    return index.data(BackupCandidates::SELECTED_ROLE).toBool();
}
