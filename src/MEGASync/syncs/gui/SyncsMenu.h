#ifndef SYNCSMENU_H
#define SYNCSMENU_H

#include "MenuItemAction.h"
#include "syncs/control/SyncSettings.h"

#include "megaapi.h"

#include <QObject>
#include <QMenu>

#include <memory>

namespace UserAttributes
{
class DeviceName;
class MyBackupsHandle;
}

class SyncsMenu : public QObject
{
    Q_OBJECT

public:
    explicit SyncsMenu(mega::MegaSync::SyncType type, QObject* parent);

    std::shared_ptr<MenuItemAction> getAction();
    std::shared_ptr<QMenu> getMenu();
    void callMenu(const QPoint& p);
    void setEnabled(bool state);

signals:
    void addSync(mega::MegaSync::SyncType type);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private slots:
    void onAddSync();
    void onDeviceNameSet(QString name);

private:
    void refresh();
    void highLightMenuEntry(QAction *action);

    QString createSyncTooltipText(std::shared_ptr<SyncSettings> syncSetting) const;

    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
    mega::MegaSync::SyncType mType;
    std::shared_ptr<QMenu> mMenu;
    std::shared_ptr<MenuItemAction> mAddAction;
    std::shared_ptr<MenuItemAction> mMenuAction;
    MenuItemAction* mLastHovered;
    std::shared_ptr<MenuItemAction> mDevNameAction;
};

#endif // SYNCSMENU_H
