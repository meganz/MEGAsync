#ifndef SYNCSMENU_H
#define SYNCSMENU_H

#include "megaapi.h"
#include "MegaMenuItemAction.h"
#include "SyncSettings.h"

#include <QMenu>
#include <QObject>
#include <QPointer>

#include <memory>

namespace UserAttributes
{
class DeviceName;
class MyBackupsHandle;
}

class SyncsMenu: public QObject
{
    Q_OBJECT

public:
    explicit SyncsMenu(mega::MegaSync::SyncType type, QWidget* parent = nullptr);

    static SyncsMenu* newSyncsMenu(mega::MegaSync::SyncType type, QWidget* parent = nullptr);
    QPointer<MegaMenuItemAction> getAction();
    void callMenu(const QPoint& p);
    void setEnabled(bool state);

signals:
    void addSync(mega::MegaSync::SyncType type);

protected:
    explicit SyncsMenu(mega::MegaSync::SyncType type, int itemIndent, QWidget* parent = nullptr);
    ~SyncsMenu();

    bool eventFilter(QObject* obj, QEvent* e) override;
    virtual QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const;
    virtual void refresh();

    QPointer<QMenu> getMenu();

    virtual QString getMenuActionText() const = 0;
    virtual QString getAddActionText() const = 0;

    QPointer<QMenu> mMenu;

private slots:
    void onAddSync();

private:
    QPointer<MegaMenuItemAction> mAddAction;
    QPointer<MegaMenuItemAction> mMenuAction;

    mega::MegaSync::SyncType mType;
    int mItemIndent;
};

class TwoWaySyncsMenu : public SyncsMenu
{
    Q_OBJECT

public:
    explicit TwoWaySyncsMenu(QWidget* parent = nullptr);

protected:
    QString getMenuActionText() const override;
    QString getAddActionText() const override;

private:
    QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const override;

    static const int mTwoWaySyncItemIndent = 0;
};

class BackupSyncsMenu : public SyncsMenu
{
    Q_OBJECT

public:
    explicit BackupSyncsMenu(QWidget* parent = nullptr);

protected:
    QString getMenuActionText() const override;
    QString getAddActionText() const override;

private slots:
    void onDeviceNameSet(QString name);

private:
    QString createSyncTooltipText(const std::shared_ptr<SyncSettings>& syncSetting) const override;
    void refresh() override;

    static const int mBackupItemIndent = 1;
    QPointer<MegaMenuItemAction> mDevNameAction;
    std::shared_ptr<UserAttributes::DeviceName> mDeviceNameRequest;
    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
};


#endif // SYNCSMENU_H
