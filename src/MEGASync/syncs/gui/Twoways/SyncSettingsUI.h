#ifndef SYNCSETTINGSUI_H
#define SYNCSETTINGSUI_H

#include <QWidget>
#include <QPointer>

#include <SyncSettingsUIBase.h>
#include <syncs/gui/Twoways/SyncTableView.h>

class SyncSettingsUI : public SyncSettingsUIBase
{
    Q_OBJECT

public:
    explicit SyncSettingsUI(QWidget *parent = nullptr);
    ~SyncSettingsUI() override;
};

#endif // SYNCSETTINGSUI_H
