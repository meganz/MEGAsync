#ifndef SYNCSETTINGSUI_H
#define SYNCSETTINGSUI_H

#include <QWidget>
#include <QPointer>

#include <SyncSettingsUIBase.h>
#include <syncs/gui/Twoways/SyncTableView.h>
#include <syncs/gui/Twoways/SyncSettingsElements.h>

class SyncSettingsUI : public SyncSettingsUIBase
{
    Q_OBJECT

public:
    explicit SyncSettingsUI(QWidget *parent = nullptr);
    ~SyncSettingsUI() override;

    void setOverQuotaMode(bool state);

protected:
    QString getFinishWarningIconString() override;
    QString getFinishIconString() override;
    QString typeString() override;

private:
    SyncSettingsElements mSyncElement;
};

#endif // SYNCSETTINGSUI_H
