#include "SyncsMenu.h"
#include "Utilities.h"
#include "Preferences.h"
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

// SyncsMenu ----------
SyncsMenu::SyncsMenu(mega::MegaSync::SyncType type,
                     const QIcon& iconMenu,
                     const QString& addActionText,
                     const QString& menuActionText,
                     QObject *parent) :
    QObject(parent),
    mMenu (new QMenu()),
    mAddAction (new MenuItemAction(QString(), QIcon(), true)),
    mMenuAction (new MenuItemAction(QString(), QIcon(), true)),
    mLastHovered (nullptr),
    mType (type),
    mMenuIcon(iconMenu),
    mAddActionText(addActionText),
    mMenuActionText(menuActionText)
{
    mAddAction->setLabelText(tr(mAddActionText.toUtf8().constData()));
    mAddAction->setParent(this);
    connect(mAddAction, &MenuItemAction::triggered,
            this, &SyncsMenu::onAddSync);

    mMenuAction->setLabelText(tr(mMenuActionText.toUtf8().constData()));
    mMenuAction->setIcon(mMenuIcon);
    mMenuAction->setParent(this);

    Platform::getInstance()->initMenu(mMenu,
                                      QString::fromLatin1("SyncsMenu - ").append(mMenuActionText).toUtf8().constData());
    mMenu->setToolTipsVisible(true);

    //Highlight menu entry on mouse over
    connect(mMenu, &QMenu::hovered,
            this, &SyncsMenu::highLightMenuEntry);
    mMenu->installEventFilter(this);
}

QPointer<MenuItemAction> SyncsMenu::getAction()
{
    refresh();
    return mMenu->actions().isEmpty() ? mAddAction : mMenuAction;
}

QPointer<QMenu> SyncsMenu::getMenu()
{
    refresh();
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
        mMenuAction->setLabelText(tr(mMenuActionText.toUtf8().constData()));
        mAddAction->setLabelText(tr(mAddActionText.toUtf8().constData()));
    }
    return QObject::eventFilter(obj, e);
}

void SyncsMenu::onAddSync()
{
    emit addSync(mType);
}

void SyncsMenu::highLightMenuEntry(QAction* action)
{
    auto pAction (qobject_cast<MenuItemAction*>(action));
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

QString SyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    QString toolTip (SyncTooltipCreator::createForLocal(syncSetting->getLocalFolder()));
    toolTip += QChar::LineSeparator;
    return toolTip;
}

// TwoWaySyncsMenu ----
TwoWaySyncsMenu::TwoWaySyncsMenu(QObject* parent) :
    SyncsMenu(mega::MegaSync::TYPE_TWOWAY,
              QIcon(QLatin1String("://images/icons/ico_sync.png")),
              QLatin1String("Add Sync"),
              QLatin1String("Syncs"),
              parent)
{
    QT_TR_NOOP("Add Sync");
    QT_TR_NOOP("Syncs");
}

void TwoWaySyncsMenu::refresh()
{
    SyncInfo* model (SyncInfo::instance());
    MenuItemAction* firstBackup (nullptr);

    // Actions will be deleted, so reset the last hovered pointer
    mLastHovered = nullptr;

    // Reset menu (leave mAddAction only)
    const auto actions (mMenu->actions());
    for (QAction* a : actions)
    {
        if (a != mAddAction)
        {
            mMenu->removeAction(a);
            delete a;
        }
    }

    if (mAddAction)
    {
        mMenu->removeAction(mAddAction);
    }

    int activeFolders (0);

    // Get number of <type>. Show only "Add <type>" button if no items, and whole menu otherwise.
    int numItems = (Preferences::instance()->logged()) ?
                       model->getNumSyncedFolders(mType)
                                                       : 0;
    for (int i = 0; i < numItems; ++i)
    {
        auto backupSetting = model->getSyncSetting(i, mType);

        if (backupSetting->isActive())
        {
            activeFolders++;
            MenuItemAction* action =
                new MenuItemAction(SyncController::getSyncNameFromPath(backupSetting->getLocalFolder(true)),
                                   QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")),
                                   true);
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

        // Display "Add <type>" at the end of the list
        if (activeFolders)
        {
            const QIcon iconAdd (QLatin1String("://images/icons/ico_add_sync.png"));
            mAddAction->setIcon(iconAdd);
            mMenu->addSeparator();
            mMenu->addAction(mAddAction);
        }
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

QString TwoWaySyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    return SyncsMenu::createSyncTooltipText(syncSetting)
           + SyncTooltipCreator::createForRemote(syncSetting->getMegaFolder());
}

// BackupSyncsMenu ----
BackupSyncsMenu::BackupSyncsMenu(QObject* parent) :
    SyncsMenu(mega::MegaSync::TYPE_BACKUP,
                QIcon(QLatin1String("://images/icons/ico_backup.png")),
                QLatin1String("Add Backup"),
                QLatin1String("Backups"),
                parent),
    mDeviceNameRequest(UserAttributes::DeviceName::requestDeviceName()),
    mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle()),
    mDevNameAction(nullptr)
{
    QT_TR_NOOP("Add Backup");
    QT_TR_NOOP("Backups");
    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &BackupSyncsMenu::onDeviceNameSet);
}

void BackupSyncsMenu::refresh()
{
    SyncInfo* model (SyncInfo::instance());
    MenuItemAction* firstBackup (nullptr);

    // Actions will be deleted, so reset the last hovered pointer
    mLastHovered = nullptr;

    // Reset menu (leave mAddAction only)
    const auto actions (mMenu->actions());
    for (QAction* a : actions)
    {
        mMenu->removeAction(a);
        if (a != mAddAction && a != mDevNameAction)
        {
            a->deleteLater();
        }
    }

    int activeFolders (0);

    // Get number of <type>. Show only "Add <type>" button if no items, and whole menu otherwise.
    int numItems = (Preferences::instance()->logged()) ?
                       model->getNumSyncedFolders(mType)
                                                       : 0;
    for (int i = 0; i < numItems; ++i)
    {
        auto backupSetting = model->getSyncSetting(i, mType);

        if (backupSetting->isActive())
        {
            activeFolders++;
            MenuItemAction* action =
                new MenuItemAction(SyncController::getSyncNameFromPath(backupSetting->getLocalFolder(true)),
                                   QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")),
                                   true, true);
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

        // Display "Add <type>" at the end of the list
        if (activeFolders)
        {
            const QIcon iconAdd (QLatin1String("://images/icons/ico_add_sync.png"));
            mAddAction->setIcon(iconAdd);
            mMenu->addSeparator();
            mMenu->addAction(mAddAction);
        }
    }

    if (!numItems || !activeFolders)
    {
        mMenuAction->setMenu(nullptr);
        mAddAction->setIcon(mMenuIcon);
    }
    else
    {
        // Show device name
        if (!mDevNameAction)
        {
            // Display device name before folders
            mDevNameAction = new MenuItemAction(QString(), QIcon(DEVICE_ICON), true);
            mDevNameAction->setParent(this);
            // Insert the action in the menu to make sure it is here when the
            // set device name slot is called.
            mMenu->insertAction(firstBackup, mDevNameAction);
            onDeviceNameSet(mDeviceNameRequest->getDeviceName());
        }
        else
        {
            mMenu->insertAction(firstBackup, mDevNameAction);
        }
        mMenuAction->setMenu(mMenu);
    }
}

QString BackupSyncsMenu::createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const
{
    return SyncsMenu::createSyncTooltipText(syncSetting)
           + SyncTooltipCreator::createForRemote(mMyBackupsHandleRequest->getNodeLocalizedPath(syncSetting->getMegaFolder()));
}

void BackupSyncsMenu::onDeviceNameSet(QString name)
{
    if (mDevNameAction)
    {
        mDevNameAction->setLabelText(name);
        // Get next action to refresh devicename
        auto actions (mMenu->actions());
        auto idx (actions.indexOf(mDevNameAction));
        auto idxNext (idx + 1);
        if (idx >= 0 && idxNext < actions.size())
        {
            mMenu->removeAction(mDevNameAction);
            mMenu->insertAction(actions.at(idxNext), mDevNameAction);
        }
    }
}
