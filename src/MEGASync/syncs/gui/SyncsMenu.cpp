#include "SyncsMenu.h"
#include "Utilities.h"
#include "InfoDialog.h"
#include "Preferences/Preferences.h"
#include "syncs/control/SyncInfo.h"
#include "Platform.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "SyncTooltipCreator.h"

#include <QtConcurrent/QtConcurrent>

#ifdef Q_OS_WINDOWS
const QLatin1String DEVICE_ICON ("://images/icons/pc/pc-win_24.png");
#elif defined(Q_OS_MAC)
const QLatin1String DEVICE_ICON ("://images/icons/pc/pc-mac_24.png");
#elif defined(Q_OS_LINUX)
const QLatin1String DEVICE_ICON ("://images/icons/pc/pc-linux_24.png");
#endif

SyncsMenu::SyncsMenu(mega::MegaSync::SyncType type, QObject *parent) : QObject(parent),
    mDeviceNameRequest (nullptr),
    mMyBackupsHandleRequest (nullptr),
    mType (type),
    mMenu (new QMenu()),
    mAddAction (new MenuItemAction(QString(), QIcon(), true)),
    mMenuAction (new MenuItemAction(QString(), QIcon(), true)),
    mLastHovered (nullptr),
    mDevNameAction (nullptr)
{
    QString textAdd;
    QString textMenu;
    QIcon iconMenu;

    switch (mType)
    {
        case mega::MegaSync::TYPE_TWOWAY:
        {
            iconMenu = QIcon(QLatin1String("://images/icons/ico_sync.png"));
            break;
        }
        case mega::MegaSync::TYPE_BACKUP:
        {
            iconMenu = QIcon(QLatin1String("://images/icons/ico_backup.png"));
            mDeviceNameRequest = UserAttributes::DeviceName::requestDeviceName();
            mMyBackupsHandleRequest = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
            connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
                    this, &SyncsMenu::onDeviceNameSet);
            break;
        }
        default:
        {
            break;
        }
    }
    mAddAction->setLabelText(getAddText());
    mAddAction->setParent(this);
    connect(mAddAction.get(), &MenuItemAction::triggered,
            this, &SyncsMenu::onAddSync);

    mMenuAction->setLabelText(getMenuText());
    mMenuAction->setIcon(iconMenu);
    mMenuAction->setParent(this);

    Platform::getInstance()->initMenu(mMenu.get(), "SyncsMenu");
    mMenu->setToolTipsVisible(true);

    //Highlight menu entry on mouse over
    connect(mMenu.get(), &QMenu::hovered,
            this, &SyncsMenu::highLightMenuEntry);
    mMenu->installEventFilter(this);
}

void SyncsMenu::refresh()
{
    SyncInfo* model (SyncInfo::instance());
    MenuItemAction* firstBackup (nullptr);

    // Actions will be deleted, so reset the last hovered pointer
    mLastHovered = nullptr;

    // Reset menu (leave mAddAction only)
    const auto actions (mMenu->actions());
    for (QAction* a : actions)
    {
        if (a != mAddAction.get() && a != mDevNameAction.get())
        {
            mMenu->removeAction(a);
            delete a;
        }
    }
    if (mDevNameAction)
    {
        mMenu->removeAction(mDevNameAction.get());
    }
    if (mAddAction)
    {
        mMenu->removeAction(mAddAction.get());
    }

    int activeFolders (0);

    // Get number of <type>. Show only "Add <type>" button if no items, and whole menu otherwise.
    int numItems = (Preferences::instance()->logged()) ?
                       model->getNumSyncedFolders(mType)
                     : 0;
    if (numItems > 0)
    {
        int itemIndent (mType == mega::MegaSync::TYPE_BACKUP);

        for (int i = 0; i < numItems; ++i)
        {
            auto backupSetting = model->getSyncSetting(i, mType);

            if (backupSetting->isActive())
            {
                activeFolders++;
                MenuItemAction* action =
                        new MenuItemAction(SyncController::getSyncNameFromPath(backupSetting->getLocalFolder(true)),
                                           QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")),
                                           true, itemIndent);
                action->setToolTip(createSyncTooltipText(backupSetting));
                connect(action, &MenuItemAction::triggered,
                        this, [backupSetting](){
                    Utilities::openUrl(
                                      QUrl::fromLocalFile(backupSetting->getLocalFolder()));
                });

                mMenu->addAction(action);
                if (!firstBackup)
                {
                    firstBackup = action;
                }
            }
        }

        // Display "Add <type>" at the end of the list
        if (activeFolders)
        {
            static const QIcon iconAdd (QLatin1String("://images/icons/ico_add_sync.png"));
            mAddAction->setIcon(iconAdd);
            mMenu->addSeparator();
            mMenu->addAction(mAddAction.get());
        }
    }

    if (!numItems || !activeFolders)
    {
        mMenuAction->setMenu(nullptr);
        switch (mType)
        {
            case mega::MegaSync::TYPE_TWOWAY:
            {
                static const QIcon iconAdd (QLatin1String("://images/icons/ico_sync.png"));
                mAddAction->setIcon(iconAdd);
                break;
            }
            case mega::MegaSync::TYPE_BACKUP:
            {
                static const QIcon iconAdd (QLatin1String("://images/icons/ico_backup.png"));
                mAddAction->setIcon(iconAdd);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        // Show device name if Backups
        if (mType == mega::MegaSync::TYPE_BACKUP)
        {
            if (!mDevNameAction)
            {
                // Display device name before folders
                mDevNameAction.reset(new MenuItemAction(QString(), QIcon(DEVICE_ICON), true));
                // Insert the action in the menu to make sure it is here when the
                // set device name slot is called.
                mMenu->insertAction(firstBackup, mDevNameAction.get());
                onDeviceNameSet(mDeviceNameRequest->getDeviceName());
            }
            else
            {
                mMenu->insertAction(firstBackup, mDevNameAction.get());
            }
        }
        mMenuAction->setMenu(mMenu.get());
    }
}

std::shared_ptr<MenuItemAction> SyncsMenu::getAction()
{
    refresh();
    return mMenu->actions().isEmpty() ? mAddAction : mMenuAction;
}

std::shared_ptr<QMenu> SyncsMenu::getMenu()
{
    refresh();
    return mMenu->actions().isEmpty() ? nullptr : mMenu;
}

void SyncsMenu::callMenu(const QPoint& p)
{
    refresh();
    if (mMenu->actions().isEmpty())
    {
        onAddSync();
    }
    else
    {
        mMenu->popup(p);
    }
}

void SyncsMenu::setEnabled(bool state)
{
    mAddAction->setEnabled(state);
}

bool SyncsMenu::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == mMenu.get() && e->type() == QEvent::Leave)
    {
        if (mLastHovered)
        {
            mLastHovered->setHighlight(false);
            mLastHovered = nullptr;
        }
        return true;
    }
    else if(obj == mMenu.get() && e->type() == QEvent::LanguageChange)
    {
        mMenuAction->setLabelText(getMenuText());
        mAddAction->setLabelText(getAddText());
    }
    return QObject::eventFilter(obj, e);
}

void SyncsMenu::onAddSync()
{
    emit addSync(mType);
}

void SyncsMenu::onDeviceNameSet(QString name)
{
    if (mDevNameAction)
    {
        mDevNameAction->setLabelText(name);
        // Get next action to refresh devicename
        auto actions (mMenu->actions());
        auto idx (actions.indexOf(mDevNameAction.get()));
        auto idxNext (idx + 1);
        if (idx >= 0 && idxNext < actions.size())
        {
            mMenu->removeAction(mDevNameAction.get());
            mMenu->insertAction(actions.at(idxNext), mDevNameAction.get());
        }
    }
}

QString SyncsMenu::getAddText()
{
    QString textAdd;
    switch (mType)
    {
        case mega::MegaSync::TYPE_TWOWAY:
        {
            textAdd = tr("Add Sync");
            break;
        }
        case mega::MegaSync::TYPE_BACKUP:
        {
            textAdd = tr("Add Backup");
            break;
        }
        default:
        {
            break;
        }
    }
    return textAdd;
}

QString SyncsMenu::getMenuText()
{
    QString textMenu;
    switch (mType)
    {
        case mega::MegaSync::TYPE_TWOWAY:
        {
            textMenu = tr("Syncs");
            break;
        }
        case mega::MegaSync::TYPE_BACKUP:
        {
            textMenu = tr("Backups");
            break;
        }
        default:
        {
            break;
        }
    }
    return textMenu;
}

void SyncsMenu::highLightMenuEntry(QAction* action)
{
    MenuItemAction* pAction (static_cast<MenuItemAction*>(action));
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

QString SyncsMenu::createSyncTooltipText(std::shared_ptr<SyncSettings> syncSetting) const
{
    QString toolTip (SyncTooltipCreator::createForLocal(syncSetting->getLocalFolder()));
    toolTip += QChar::LineSeparator;
    switch (mType)
    {
    case mega::MegaSync::TYPE_TWOWAY:
    {
        toolTip += SyncTooltipCreator::createForRemote(syncSetting->getMegaFolder());
        break;
    }
    case mega::MegaSync::TYPE_BACKUP:
    {
        toolTip += SyncTooltipCreator::createForRemote(
                    mMyBackupsHandleRequest->getNodeLocalizedPath(syncSetting->getMegaFolder()));
        break;
    }
    }
    return toolTip;
}
