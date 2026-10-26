#ifndef SYNC_SETTINGS_QUICK_WIDGET_H
#define SYNC_SETTINGS_QUICK_WIDGET_H

#include "SettingsQuickWidgetBase.h"

class SyncSettingsQuickWidget: public SettingsQuickWidgetBase
{
    Q_OBJECT

public:
    explicit SyncSettingsQuickWidget(QWidget* parent = nullptr);
};

#endif // SYNC_SETTINGS_QUICK_WIDGET_H
