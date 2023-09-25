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

class SyncsMenu : public QWidget
{
    Q_OBJECT

public:
    ~SyncsMenu() override = default;

    QPointer<MenuItemAction> getAction();
    void callMenu(const QPoint& p);
    void setEnabled(bool state);

signals:
    void addSync(mega::MegaSync::SyncType type);

protected:
    explicit SyncsMenu(mega::MegaSync::SyncType type,
                       int itemIndent,
                       const QIcon& iconMenu,
                       const QString& addActionText,
                       const QString& menuActionText,
                       QWidget* parent);
    bool eventFilter(QObject* obj, QEvent* e) override;

    virtual void refresh();
    virtual QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const;
    QPointer<QMenu> getMenu();


private slots:
    void onAddSync();

private:
    void highLightMenuEntry(QAction *action);

    QPointer<MenuItemAction> mAddAction;
    MenuItemAction* mLastHovered;
    mega::MegaSync::SyncType mType;
    const int mItemIndent;
    QIcon mMenuIcon;
    QPointer<QMenu> mMenu;
    QPointer<MenuItemAction> mMenuAction;

    QString mAddActionText;
    QString mMenuActionText;
};

class TwoWaySyncsMenu : public SyncsMenu
{
    Q_OBJECT

public:
    explicit TwoWaySyncsMenu(QWidget* parent);
    ~TwoWaySyncsMenu() override = default;

private:
    QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const override;
    static const int mItemIndent = 0;
};

class BackupSyncsMenu : public SyncsMenu
{
    Q_OBJECT

public:
    explicit BackupSyncsMenu(QWidget* parent);
    ~BackupSyncsMenu() override = default;

private:
    void refresh() override;
    QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const override;
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
    QPointer<MenuItemAction> mDevNameAction;
    static const int mItemIndent = 1;

private slots:
    void onDeviceNameSet(QString name);
};

#endif // SYNCSMENU_H
