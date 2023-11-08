#ifndef SYNCSETTINGSUI_H
#define SYNCSETTINGSUI_H

#include <QWidget>
#include <QPointer>

#include <syncs/gui/SyncSettingsUIBase.h>
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
    QString disableString() override;

    //Operation failed
    QString getOperationFailTitle() override;
    QString getOperationFailText(std::shared_ptr<SyncSettings> sync) override;

    //Error adding
    QString getErrorAddingTitle() override;

    //Error removing
    QString getErrorRemovingTitle() override;
    QString getErrorRemovingText(std::shared_ptr<mega::MegaError> err) override;

private slots:
    void storageStateChanged(int newStorageState);

private:
    SyncSettingsElements mSyncElement;
};

#endif // SYNCSETTINGSUI_H
