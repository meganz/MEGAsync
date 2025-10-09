#include "SyncsMenu.h"

#include "DeviceName.h"
#include "MegaMenuItemAction.h"
#include "MyBackupsHandle.h"
#include "Preferences.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "SyncTooltipCreator.h"
#include "Utilities.h"

#include <QCoreApplication>
#include <QUrl>

const QLatin1String DEVICE_ICON("monitor");
const QLatin1String SYNC_ICON("sync-01");
const QLatin1String SYNC_ADD_ICON("sync-plus");
const QLatin1String BACKUP_ICON("database");
const QLatin1String BACKUP_ADD_ICON("database-plus");
const QLatin1String FOLDER_ICON("folder");
const QString ADD_BACKUP = QCoreApplication::translate("BackupSyncsMenu", "Add Backup");
const QString ADD_SYNC = QCoreApplication::translate("TwoWaySyncsMenu", "Add Sync");
const QString BACKUPS = QCoreApplication::translate("BackupSyncsMenu", "Backups");
const QString SYNCS = QCoreApplication::translate("TwoWaySyncsMenu", "Syncs");

SyncsMenu::SyncsMenu(mega::MegaSync::SyncType type, int itemIndent, QWidget* parent):
    QObject(parent),
    mMenu(new QMenu(parent)),
    mType(type),
    mItemIndent(itemIndent)
{
    mMenu->setProperty("class", QLatin1String("MegaMenu"));
    mMenu->setProperty("icon-token", QLatin1String("icon-primary"));
    mAddAction = new MegaMenuItemAction(
        getAddActionText(),
        Utilities::getPixmapName(type == mega::MegaSync::SyncType::TYPE_BACKUP ? BACKUP_ADD_ICON :
                                                                                 SYNC_ADD_ICON,
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false),
        0,
        mMenu);
    connect(mAddAction, &MegaMenuItemAction::triggered, this, &SyncsMenu::onAddSync);

    mMenuAction = new MegaMenuItemAction(
        getMenuActionText(),
        Utilities::getPixmapName(type == mega::MegaSync::SyncType::TYPE_BACKUP ? BACKUP_ICON :
                                                                                 SYNC_ICON,
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false),
        0);
    mMenuAction->setMenu(mMenu);

    mMenu->setToolTipsVisible(true);
    mMenu->installEventFilter(this);
}

SyncsMenu::~SyncsMenu()
{
    mMenu->deleteLater();
}

SyncsMenu* SyncsMenu::newSyncsMenu(mega::MegaSync::SyncType type, QWidget* parent)
{
    SyncsMenu* menu(nullptr);

    switch (type)
    {
        case mega::MegaSync::TYPE_TWOWAY:
        {
            menu = new TwoWaySyncsMenu(parent);
            break;
        }
        case mega::MegaSync::TYPE_BACKUP:
        {
            menu = new BackupSyncsMenu(parent);
            break;
        }
        default:
        {
            break;
        }
    }
    return menu;
}

QPointer<MegaMenuItemAction> SyncsMenu::getAction()
{
    refresh();
    auto actions = mMenu->actions();

    return actions.isEmpty() ? mAddAction : mMenuAction;
}

QPointer<QMenu> SyncsMenu::getMenu()
{
    return mMenu->actions().isEmpty() ? nullptr : mMenu;
}

void SyncsMenu::callMenu(const QPoint& p)
{
    refresh();
    mMenu->actions().isEmpty() ? onAddSync() : mMenu->popup(p);
    mAddAction->setLabelText(getAddActionText());
}

void SyncsMenu::setEnabled(bool state)
{
    mAddAction->setEnabled(state);
}

bool SyncsMenu::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == mMenu && e->type() == QEvent::LanguageChange)
    {
        mMenuAction->setLabelText(getMenuActionText());
        mAddAction->setLabelText(getAddActionText());
    }
    return QObject::eventFilter(obj, e);
}

void SyncsMenu::refresh()
{
    auto* model (SyncInfo::instance());

    // // Actions will be deleted, so reset the last hovered pointer

    // Reset menu (leave actionsToKeep)
    const auto actions (mMenu->actions());
    for (QAction* a : actions)
    {
        mMenu->removeAction(a);
        if (a != mAddAction)
        {
            a->deleteLater();
        }
    }

    int activeFolders (0);

    // Get number of <type>. Show only "Add <type>" button if no items, and whole menu otherwise.
    const int numItems = (Preferences::instance()->logged()) ?
                             model->getNumSyncedFolders(mType)
                                                             : 0;
    for (int i = 0; i < numItems; ++i)
    {
        auto syncSetting = model->getSyncSetting(i, mType);

        if (syncSetting->isActive())
        {
            activeFolders++;
            auto* action = new MegaMenuItemAction(
                SyncController::instance().getSyncNameFromPath(syncSetting->getLocalFolder(true)),
                Utilities::getPixmapName(FOLDER_ICON,
                                         Utilities::AttributeType::SMALL |
                                             Utilities::AttributeType::THIN |
                                             Utilities::AttributeType::OUTLINE,
                                         false),
                mItemIndent,
                mMenu);

            action->setToolTip(createSyncTooltipText(syncSetting));
            connect(action,
                    &MegaMenuItemAction::triggered,
                    this,
                    [syncSetting]()
                    {
                        Utilities::openUrl(
                            QUrl::fromLocalFile(syncSetting->getLocalFolder()));
                    });
            mMenu->addAction(action);
        }
    }

    // Display "Add <type>" at the end of the list
    if (activeFolders)
    {
        mMenu->addSeparator();
        mMenu->addAction(mAddAction);
    }

    if (!numItems || !activeFolders)
    {
        mMenuAction->setMenu(nullptr);
    }
    else
    {
        mMenuAction->setMenu(mMenu);
    }
}

QString SyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    QString toolTip (SyncTooltipCreator::createForLocal(syncSetting->getLocalFolder()));
    toolTip += QChar::LineSeparator;
    return toolTip;
}

void SyncsMenu::onAddSync()
{
    emit addSync(mType);
}

QString SyncsMenu::getMenuActionText() const
{
    return mType == mega::MegaSync::SyncType::TYPE_BACKUP ?
               QCoreApplication::translate("BackupSyncsMenu", "Backups") :
               QCoreApplication::translate("TwoWaySyncsMenu", "Syncs");
}

QString SyncsMenu::getAddActionText() const
{
    return mType == mega::MegaSync::SyncType::TYPE_BACKUP ?
               QCoreApplication::translate("BackupSyncsMenu", "Add Backup") :
               QCoreApplication::translate("TwoWaySyncsMenu", "Add Sync");
}

// TwoWaySyncsMenu ----
TwoWaySyncsMenu::TwoWaySyncsMenu(QWidget* parent):
    SyncsMenu(mega::MegaSync::TYPE_TWOWAY, mTwoWaySyncItemIndent, parent)
{}

QString TwoWaySyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    return SyncsMenu::createSyncTooltipText(syncSetting)
           + SyncTooltipCreator::createForRemote(syncSetting->getMegaFolder());
}

// BackupSyncsMenu ----
BackupSyncsMenu::BackupSyncsMenu(QWidget* parent):
    SyncsMenu(mega::MegaSync::TYPE_BACKUP, mBackupItemIndent, parent),
    mDevNameAction(nullptr),
    mDeviceNameRequest(UserAttributes::DeviceName::requestDeviceName()),
    mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle())
{
    connect(mDeviceNameRequest.get(),
            &UserAttributes::DeviceName::attributeReady,
            this,
            &BackupSyncsMenu::onDeviceNameSet);
}

void BackupSyncsMenu::onDeviceNameSet(QString name)
{
    auto menu (getMenu());

    if (menu && mDevNameAction)
    {
        mDevNameAction->setLabelText(name);
        // Get next action to refresh devicename
        auto actions (menu->actions());
        auto idx (actions.indexOf(mDevNameAction));
        auto idxNext (idx + 1);
        if (idx >= 0 && idxNext < actions.size())
        {
            menu->removeAction(mDevNameAction);
            menu->insertAction(actions.at(idxNext), mDevNameAction);
        }
    }
}

QString BackupSyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    return SyncsMenu::createSyncTooltipText(syncSetting)
           + SyncTooltipCreator::createForRemote(mMyBackupsHandleRequest->getNodeLocalizedPath(syncSetting->getMegaFolder()));
}

void BackupSyncsMenu::refresh()
{
    SyncsMenu::refresh();
    auto menu (getMenu());
    if (menu)
    {
        const auto actions (menu->actions());
        auto* const firstBackup (actions.isEmpty() ? nullptr : actions.first());

        // Show device name
        mDevNameAction->deleteLater();
        // Display device name before folders
        mDevNameAction =
            new MegaMenuItemAction(QString(),
                                   Utilities::getPixmapName(DEVICE_ICON,
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false),
                                   0,
                                   menu);
        menu->insertAction(firstBackup, mDevNameAction);
        onDeviceNameSet(mDeviceNameRequest->getDeviceName());
    }
}
