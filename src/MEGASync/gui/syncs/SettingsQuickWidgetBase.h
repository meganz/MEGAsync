#ifndef SETTINGS_QUICK_WIDGET_BASE_H
#define SETTINGS_QUICK_WIDGET_BASE_H

#include "MegaQuickWidget.h"

class SyncController;
class SyncSettingsModelBase;

/**
 * @brief Common bridge widget shared by the Syncs and Backups settings tabs.
 *
 * Owns the list model and a reference to the relevant controller (SyncController for
 * syncs, BackupsController for backups; the latter is-a SyncController, so the shared
 * pause/resume/reboot/remove operations dispatch through the base reference). The QML
 * delegate calls into the tab-neutral Q_INVOKABLEs declared here; the two genuinely
 * tab-specific operations (add and remove, which use different create/remove managers)
 * are virtual seams implemented by the subclasses.
 */
class SettingsQuickWidgetBase: public MegaQuickWidget
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

    Q_INVOKABLE void setContextMenuOpen(bool open);

signals:
    void closeContextMenu();

protected:
    SettingsQuickWidgetBase(SyncSettingsModelBase* model,
                            SyncController& controller,
                            QWidget* parent = nullptr);

    SyncSettingsModelBase* model() const;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    SyncSettingsModelBase* mModel;
    SyncController& mController;
};

#endif // SETTINGS_QUICK_WIDGET_BASE_H
