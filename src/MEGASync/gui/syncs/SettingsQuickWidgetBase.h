#ifndef SETTINGS_QUICK_WIDGET_BASE_H
#define SETTINGS_QUICK_WIDGET_BASE_H

#include "MegaQuickWidget.h"

class SyncSettingsActionsBase;

/**
 * @brief Legacy Settings dialog host of the Syncs and Backups QML surfaces.
 *
 * The actions QML calls live in SyncSettingsActionsBase and its subclasses
 * (SNC-6795). This widget only hosts the QML view, exposes the actions and the
 * model to it, and closes the delegate's context menu on clicks outside the view.
 */
class SettingsQuickWidgetBase: public MegaQuickWidget
{
    Q_OBJECT

public:
    SyncSettingsActionsBase* actions() const;

protected:
    explicit SettingsQuickWidgetBase(QWidget* parent = nullptr);

    // Called once by the subclass constructor before setSource(). The actions
    // object must already be parented to this widget.
    void setActions(SyncSettingsActionsBase* actions);
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void onContextMenuOpenChanged(bool open);

    SyncSettingsActionsBase* mActions = nullptr;
};

#endif // SETTINGS_QUICK_WIDGET_BASE_H
