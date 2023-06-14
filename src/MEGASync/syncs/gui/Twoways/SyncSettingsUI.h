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

protected:
    QString getFinishWarningIconString() override;
    QString getFinishIconString() override;
    QString typeString() override;
    QString disableString() override;

private slots:
    void storageStateChanged(int newStorageState);

private:
    SyncSettingsElements mSyncElement;
};

#endif // SYNCSETTINGSUI_H
