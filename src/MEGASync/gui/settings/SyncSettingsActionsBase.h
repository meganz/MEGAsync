#ifndef SYNC_SETTINGS_ACTIONS_BASE_H
#define SYNC_SETTINGS_ACTIONS_BASE_H

#include <QObject>
#include <QPointer>
#include <QString>

class QWidget;
class SyncController;
class SyncSettingsModelBase;

// Tab-neutral actions shared by the
// Syncs and Backups settings surfaces, extracted from SettingsQuickWidgetBase so
// they no longer depend on QQuickWidget. Owns the list model and a reference to
// the relevant controller (SyncController for syncs, BackupsController for
// backups, which is-a SyncController). QML reaches this object as
// `settingsAccess` in the shared delegates and as `syncSettingsAccess` /
// `backupSettingsAccess` in the tab surfaces.
//
// `hostWidget` is the widget that visually hosts the surface (today the
// QQuickWidget, tomorrow the QML window wrapper). Dialogs opened from here are
// parented to its window() / parentWidget(), resolved at call time exactly as
// the widget used to do.
class SyncSettingsActionsBase: public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void exploreLocal(const QString& localFolder) const;
    Q_INVOKABLE void openInMega(int index) const;
    Q_INVOKABLE void pause(int index) const;
    Q_INVOKABLE void resume(int index) const;
    Q_INVOKABLE void openExclusionsDialog(int index) const;
    Q_INVOKABLE void rescan(int index) const;
    Q_INVOKABLE void reboot(int index) const;
    Q_INVOKABLE void removeNonConfirmation(int index) const;
    Q_INVOKABLE void onGetMoreQuotaClicked() const;
    Q_INVOKABLE void sortModelByName(bool ascending = true);
    Q_INVOKABLE void sortModelByStatus(bool ascending = true);

    Q_INVOKABLE virtual void addItem() const = 0;
    Q_INVOKABLE virtual void remove(int index) const = 0;

    // The delegate reports its context menu opening/closing; the host decides
    // how to detect outside clicks and answers through requestCloseContextMenu().
    Q_INVOKABLE void setContextMenuOpen(bool open);
    void requestCloseContextMenu();

    SyncSettingsModelBase* model() const;

signals:
    void closeContextMenu();
    void contextMenuOpenChanged(bool open);

protected:
    SyncSettingsActionsBase(SyncSettingsModelBase* model,
                            SyncController& controller,
                            QWidget* hostWidget,
                            QObject* parent = nullptr);

    QWidget* hostWindow() const;
    QWidget* hostParentWidget() const;

private:
    SyncSettingsModelBase* mModel;
    SyncController& mController;
    QPointer<QWidget> mHostWidget;
};

#endif // SYNC_SETTINGS_ACTIONS_BASE_H
