#ifndef BACKUP_SETTINGS_QUICK_WIDGET_H
#define BACKUP_SETTINGS_QUICK_WIDGET_H

#include "SettingsQuickWidgetBase.h"

class BackupSettingsQuickWidget: public SettingsQuickWidgetBase
{
    Q_OBJECT

public:
    explicit BackupSettingsQuickWidget(QWidget* parent = nullptr);
};

#endif // BACKUP_SETTINGS_QUICK_WIDGET_H
