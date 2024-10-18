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
    return index.data(BackupCandidates::SELECTED_ROLE).toBool();
}
