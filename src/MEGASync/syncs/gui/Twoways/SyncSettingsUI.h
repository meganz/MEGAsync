#ifndef SYNCSETTINGSUI_H
#define SYNCSETTINGSUI_H

#include <QWidget>

#include "SyncSettingsUIBase.h"
#include "SyncSettingsElements.h"

class SyncSettingsUI : public SyncSettingsUIBase
{
    Q_OBJECT

public:
    explicit SyncSettingsUI(QWidget *parent = nullptr);
    ~SyncSettingsUI() override = default;

    void addButtonClicked(mega::MegaHandle megaFolderHandle = mega::INVALID_HANDLE) override;

protected:
    QString getFinishWarningIconString() const override;
    QString getFinishIconString() const override;
    QString disableString() const override;

    //Operation failed
    QString getOperationFailTitle() const override;
    QString getOperationFailText(std::shared_ptr<SyncSettings> sync) override;

    //Error adding
    QString getErrorAddingTitle() const override;

    //Error removing
    QString getErrorRemovingTitle() const override;
    QString getErrorRemovingText(std::shared_ptr<mega::MegaError> err) override;

    void removeSync(std::shared_ptr<SyncSettings> sync) override;

    void setSyncsTitle();
    void changeEvent(QEvent *) override;

private slots:
    void storageStateChanged(int newStorageState);

private:
    SyncSettingsElements mSyncElement;
};

#endif // SYNCSETTINGSUI_H
