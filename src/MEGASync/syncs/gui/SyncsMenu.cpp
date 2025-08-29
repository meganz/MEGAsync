#include "SyncsMenu.h"

#include "DeviceName.h"
#include "MyBackupsHandle.h"
#include "Platform.h"
#include "Preferences.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "SyncTooltipCreator.h"
#include "Utilities.h"

const QLatin1String DEVICE_ICON("://monitor.svg");
const QLatin1String SYNC_ICON("://sync-01.svg");
const QLatin1String SYNC_ADD_ICON("://sync-plus.svg");
const QLatin1String BACKUPC_ICON("://database.svg");
const QLatin1String BACKUP_ADD_ICON("://database-plus.svg");
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
    mAddAction = new MegaMenuItemAction(
        type == mega::MegaSync::SyncType::TYPE_BACKUP ? ADD_BACKUP : ADD_SYNC,
        QLatin1String(type == mega::MegaSync::SyncType::TYPE_BACKUP ? BACKUP_ADD_ICON :
                                                                      SYNC_ADD_ICON),
        0,
        mMenu);
    connect(mAddAction, &MenuItemAction::triggered,
            this, &SyncsMenu::onAddSync);

    mMenuAction = new MegaMenuItemAction(
        type == mega::MegaSync::SyncType::TYPE_BACKUP ? BACKUPS : SYNCS,
        QLatin1String(type == mega::MegaSync::SyncType::TYPE_BACKUP ? BACKUPC_ICON : SYNC_ICON),
        0);
    mMenuAction->setSubmenu(mMenu);
    mMenu->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    mMenu->setAttribute(Qt::WA_TranslucentBackground);

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
                QLatin1String("://folder.svg"),
                mItemIndent,
                mMenu);

            action->setToolTip(createSyncTooltipText(syncSetting));
            connect(action, &MenuItemAction::triggered,
                    this, [syncSetting](){
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
        mMenuAction->setSubmenu(nullptr);
    }
    else
    {
        mMenuAction->setSubmenu(mMenu);
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

// TwoWaySyncsMenu ----
TwoWaySyncsMenu::TwoWaySyncsMenu(QWidget* parent):
    SyncsMenu(mega::MegaSync::TYPE_TWOWAY, mTwoWaySyncItemIndent, parent)
{}

QString TwoWaySyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    return SyncsMenu::createSyncTooltipText(syncSetting)
           + SyncTooltipCreator::createForRemote(syncSetting->getMegaFolder());
}

QString TwoWaySyncsMenu::getMenuActionText() const
{
    return tr("Syncs");
}

QString TwoWaySyncsMenu::getAddActionText() const
{
    return tr("Add Sync");
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
        mDevNameAction = new MegaMenuItemAction(QString(), DEVICE_ICON, 0, menu);
        menu->insertAction(firstBackup, mDevNameAction);
        onDeviceNameSet(mDeviceNameRequest->getDeviceName());
    }
}

QString BackupSyncsMenu::getMenuActionText() const
{
    return tr("Backups");
}

QString BackupSyncsMenu::getAddActionText() const
{
    return tr("Add Backup");
}
