#ifndef SYNCSMENU_H
#define SYNCSMENU_H

#include "MenuItemAction.h"
#include "SyncController.h"

#include "megaapi.h"

#include <QObject>
#include <QMenu>

#include <memory>

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

    SyncController mSyncController;
    mega::MegaSync::SyncType mType;
    std::shared_ptr<QMenu> mMenu;
    std::shared_ptr<MenuItemAction> mAddAction;
    std::shared_ptr<MenuItemAction> mMenuAction;
    MenuItemAction* mLastHovered;
    std::shared_ptr<MenuItemAction> mDevNameAction;
};

#endif // SYNCSMENU_H
