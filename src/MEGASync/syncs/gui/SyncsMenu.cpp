#include "SyncsMenu.h"
#include "Utilities.h"
//#include "InfoDialog.h"
#include "Preferences/Preferences.h"
#include "syncs/control/SyncInfo.h"
#include "Platform.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "SyncTooltipCreator.h"

#ifdef Q_OS_WINDOWS
const QLatin1String DEVICE_ICON ("://images/icons/pc/pc-win_24.png");
#elif defined(Q_OS_MAC)
const QLatin1String DEVICE_ICON ("://images/icons/pc/pc-mac_24.png");
#elif defined(Q_OS_LINUX)
const QLatin1String DEVICE_ICON ("://images/icons/pc/pc-linux_24.png");
#endif

// SyncsMenu ----------
SyncsMenu::SyncsMenu(mega::MegaSync::SyncType type,
                     int itemIndent,
                     const QIcon& iconMenu,
                     QWidget *parent)
    : QWidget(parent)
    , mMenu (new QMenu(this))
    , mLastHovered (nullptr)
    , mType (type)
    , mItemIndent (itemIndent)
    , mMenuIcon(iconMenu)
{
    mAddAction = new MenuItemAction(QString(), QString(), mMenu);
    mAddAction->setManagesHoverStates(true);
    connect(mAddAction, &MenuItemAction::triggered,
            this, &SyncsMenu::onAddSync);

    mMenuAction = new MenuItemAction(QString(), QString(), mMenu);
    mMenuAction->setManagesHoverStates(true);
    mMenuAction->setIcon(mMenuIcon);

    mMenu->setToolTipsVisible(true);

    //Highlight menu entry on mouse over
    connect(mMenu, &QMenu::hovered,
            this, &SyncsMenu::highLightMenuEntry);
    mMenu->installEventFilter(this);
}

SyncsMenu* SyncsMenu::newSyncsMenu(mega::MegaSync::SyncType type, bool isEnabled, QWidget* parent)
{
    SyncsMenu* menu (nullptr);

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
    }

    if (menu)
    {
        menu->setEnabled(isEnabled);
    }
    return menu;
}

QPointer<MenuItemAction> SyncsMenu::getAction()
{
    refresh();
    return mMenu->actions().isEmpty() ? mAddAction : mMenuAction;
}

QPointer<QMenu> SyncsMenu::getMenu()
{
    return mMenu->actions().isEmpty() ? nullptr : mMenu;
}

void SyncsMenu::callMenu(const QPoint& p)
{
    refresh();
    mMenu->actions().isEmpty() ? onAddSync() : mMenu->popup(p);
}

void SyncsMenu::setEnabled(bool state)
{
    mAddAction->setEnabled(state);
}

void SyncsMenu::initMenuTexts()
{
    mAddAction->setLabelText(getAddActionText());
    mMenuAction->setLabelText(getMenuActionText());
    const auto menuName (QString::fromLatin1("SyncsMenu - ").append(getMenuActionText()));
    Platform::getInstance()->initMenu(mMenu, menuName.toUtf8().constData());
}

bool SyncsMenu::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == mMenu && e->type() == QEvent::Leave)
    {
        if (mLastHovered)
        {
            mLastHovered->setHighlight(false);
            mLastHovered = nullptr;
        }
        return true;
    }
    else if(obj == mMenu && e->type() == QEvent::LanguageChange)
    {
        mMenuAction->setLabelText(getMenuActionText());
        mAddAction->setLabelText(getAddActionText());
    }
    return QObject::eventFilter(obj, e);
}

void SyncsMenu::refresh()
{
    auto* model (SyncInfo::instance());

    // Actions will be deleted, so reset the last hovered pointer
    mLastHovered = nullptr;

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
            auto* action =
                new MenuItemAction(SyncController::getSyncNameFromPath(syncSetting->getLocalFolder(true)),
                                   QLatin1String("://images/icons/folder/folder-mono_24.png"),
                                   mMenu);
            action->setManagesHoverStates(true);
            action->setTreeDepth(mItemIndent);
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
        const QIcon iconAdd (QLatin1String("://images/icons/ico_add_sync.png"));
        mAddAction->setIcon(iconAdd);
        mMenu->addSeparator();
        mMenu->addAction(mAddAction);
    }

    if (!numItems || !activeFolders)
    {
        mMenuAction->setMenu(nullptr);
        mAddAction->setIcon(mMenuIcon);
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

void SyncsMenu::highLightMenuEntry(QAction* action)
{
    auto* pAction (qobject_cast<MenuItemAction*>(action));
    if (pAction)
    {
        if (mLastHovered)
        {
            mLastHovered->setHighlight(false);
        }
        pAction->setHighlight(true);
        mLastHovered = pAction;
    }
}

// TwoWaySyncsMenu ----
TwoWaySyncsMenu::TwoWaySyncsMenu(QWidget* parent)
    : SyncsMenu(mega::MegaSync::TYPE_TWOWAY,
                    mTwoWaySyncItemIndent,
                    QIcon(QLatin1String("://images/icons/ico_sync.png")),
                    parent)
{
    initMenuTexts();
}

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
BackupSyncsMenu::BackupSyncsMenu(QWidget* parent)
    : SyncsMenu(mega::MegaSync::TYPE_BACKUP,
                    mBackupItemIndent,
                    QIcon(QLatin1String("://images/icons/ico_backup.png")),
                    parent)
    , mDevNameAction(nullptr)
    , mDeviceNameRequest(UserAttributes::DeviceName::requestDeviceName())
    , mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle())
{
    initMenuTexts();
    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &BackupSyncsMenu::onDeviceNameSet);
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
        mDevNameAction = new MenuItemAction(QString(), DEVICE_ICON, menu);
        mDevNameAction->setManagesHoverStates(true);
        // Insert the action in the menu to make sure it is here when the
        // set device name slot is called.
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
