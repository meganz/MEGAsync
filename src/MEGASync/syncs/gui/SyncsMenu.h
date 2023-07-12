#ifndef SYNCSMENU_H
#define SYNCSMENU_H

#include "MenuItemAction.h"
#include "syncs/control/SyncSettings.h"

#include "megaapi.h"

#include <QObject>
#include <QMenu>
#include <QPointer>

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
    ~SyncsMenu(){}

    QPointer<MenuItemAction> getAction();
    QPointer<QMenu> getMenu();
    void callMenu(const QPoint& p);
    void setEnabled(bool state);

signals:
    void addSync(mega::MegaSync::SyncType type);

protected:
    explicit SyncsMenu(mega::MegaSync::SyncType type,
                       const QIcon& iconMenu,
                       const QString& addActionText,
                       const QString& menuActionText,
                       QObject* parent);
    bool eventFilter(QObject* obj, QEvent* e) override;
    virtual void refresh() = 0;
    virtual QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const;

    QPointer<QMenu> mMenu;
    QPointer<MenuItemAction> mAddAction;
    QPointer<MenuItemAction> mMenuAction;
    MenuItemAction* mLastHovered;
    mega::MegaSync::SyncType mType;
    QIcon mMenuIcon;

private slots:
    void onAddSync();

private:
    void highLightMenuEntry(QAction *action);
    QString mAddActionText;
    QString mMenuActionText;
};

class TwoWaySyncsMenu : public SyncsMenu
{
    Q_OBJECT

public:
    explicit TwoWaySyncsMenu(QObject* parent);
    ~TwoWaySyncsMenu() {}

private:
    void refresh() override;
    QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const override;
};

class BackupSyncsMenu : public SyncsMenu
{
    Q_OBJECT

public:
    explicit BackupSyncsMenu(QObject* parent);
    ~BackupSyncsMenu() {}

private:
    void refresh() override;
    QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const override;

private slots:
    void onDeviceNameSet(QString name);

private:
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
    QPointer<MenuItemAction> mDevNameAction;
};

#endif // SYNCSMENU_H
