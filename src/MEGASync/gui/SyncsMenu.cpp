#include "SyncsMenu.h"
#include "Utilities.h"
#include "InfoDialog.h"
#include "Preferences.h"
#include "model/SyncModel.h"

#include <QtConcurrent/QtConcurrent>

#ifdef __APPLE__
const QLatin1String MENU_STYLESHEET ("QMenu {background: #ffffff;"
                                        "padding-top: 8px; "
                                        "padding-bottom: 8px;}");
#else
const QLatin1String MENU_STYLESHEET ("QMenu {border: 1px solid #B8B8B8;"
                                        "border-radius: 5px;"
                                        "background: #ffffff;"
                                        "padding-top: 8px;"
                                        "padding-bottom: 8px;}");
#endif

#ifdef WIN32
const QLatin1String DEVICE_ICON ("://images/PC_win_ico_rest.png");
#elif defined(__APPLE__)
const QLatin1String DEVICE_ICON ("://images/PC_mac_ico_rest.png");
#elif defined(Q_OS_LINUX)
const QLatin1String DEVICE_ICON ("://images/PC_linux_ico_rest.png");
#endif

SyncsMenu::SyncsMenu(mega::MegaSync::SyncType type, QObject *parent) : QObject(parent),
    mType (type),
    mMenu (new QMenu()),
    mAddAction (new MenuItemAction(QString(), QIcon(), true)),
    mMenuAction (new MenuItemAction(QString(), QIcon(), true)),
    mLastHovered (nullptr),
    mDevNameAction (nullptr)
{
    QString textAdd;
    QString textMenu;
    QIcon iconAdd;
    QIcon iconMenu;

    switch (mType)
    {
        case mega::MegaSync::TYPE_TWOWAY:
        {
            textAdd = tr("Add Sync");
            iconAdd = QIcon(QLatin1String("://images/ico_drop_add_sync.png"));
            textMenu = tr("Syncs");
            iconMenu = QIcon(QLatin1String("://images/ico_add_sync_folder.png"));
            break;
        }
        case mega::MegaSync::TYPE_BACKUP:
        {
            textAdd = tr("Add Backups");
            iconAdd = QIcon(QLatin1String("://images/Backup.png"));
            textMenu = tr("Backups");
            iconMenu = iconAdd;
            connect(&SyncController::instance(), &SyncController::deviceName,
                    this, &SyncsMenu::onDeviceNameSet);
            break;
        }
        default:
        {
            break;
        }
    }
    mAddAction->setLabelText(textAdd);
    mAddAction->setIcon(iconAdd);
    mAddAction->setParent(this);
    connect(mAddAction.get(), &MenuItemAction::triggered,
            this, &SyncsMenu::onAddSync, Qt::QueuedConnection);

    mMenuAction->setLabelText(textMenu);
    mMenuAction->setIcon(iconMenu);
    mMenuAction->setParent(this);

    mMenu->setStyleSheet(MENU_STYLESHEET);

    // Do not display broken menu shadow in windows
#ifdef _WIN32
    mMenu->setAttribute(Qt::WA_TranslucentBackground);
    mMenu->setWindowFlags(mMenu->windowFlags()
                          | Qt::FramelessWindowHint
                          | Qt::NoDropShadowWindowHint);
#endif

    //Highlight menu entry on mouse over
    connect(mMenu.get(), &QMenu::hovered,
            this, &SyncsMenu::highLightMenuEntry, Qt::QueuedConnection);
    mMenu->installEventFilter(this);
}

void SyncsMenu::refresh()
{
    Preferences* preferences (Preferences::instance());
    SyncModel* model (SyncModel::instance());
    MenuItemAction* firstBackup (nullptr);

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
    int numItems = (preferences->logged()) ?
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
                        new MenuItemAction(backupSetting->name(),
                                           QIcon(QLatin1String("://images/small_folder.png")),
                                           true, itemIndent);
                connect(action, &MenuItemAction::triggered,
                        this, [backupSetting](){
                    QtConcurrent::run(QDesktopServices::openUrl,
                                      QUrl::fromLocalFile(backupSetting->getLocalFolder()));
                }, Qt::QueuedConnection);

                mMenu->addAction(action);
                if (!firstBackup)
                {
                    firstBackup = action;
                }
            }
        }

        // Display "Add <type>" only if the whole remote / is not synced...
        if (activeFolders && !model->isRemoteRootSynced())
        {
            mMenu->addSeparator();
            mMenu->addAction(mAddAction.get());
        }
    }

    if (!numItems || !activeFolders)
    {
        mMenuAction->setMenu(nullptr);
    }
    else
    {
        // Show device name if Backups
        if (mType == mega::MegaSync::TYPE_BACKUP)
        {
            if (!mDevNameAction)
            {
                // Display device name before folders (click opens backups wizard)
                mDevNameAction.reset(new MenuItemAction(QString(), QIcon(DEVICE_ICON), true));
                mMenu->insertAction(firstBackup, mDevNameAction.get());
                // Insert the action in the menu to make sure it is here when the
                // set device name slot is called.
                connect(mDevNameAction.get(), &MenuItemAction::triggered,
                        this, &SyncsMenu::onAddSync, Qt::QueuedConnection);
                SyncController::instance().getDeviceName();
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
