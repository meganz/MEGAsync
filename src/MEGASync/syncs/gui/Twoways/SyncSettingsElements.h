#ifndef SYNCSETTINGSELEMENTS_H
#define SYNCSETTINGSELEMENTS_H

#include <QWidget>

namespace Ui {
class SyncAccountFullMessage;
class SyncStallModeSelector;
}

class SyncSettingsUIBase;

class SyncSettingsElements : public QObject
{
    Q_OBJECT

public:
    explicit SyncSettingsElements(QObject *parent = nullptr);
    ~SyncSettingsElements();

    void initElements(SyncSettingsUIBase* syncSettingsUi);
    void setOverQuotaMode(bool state);

private slots:
    void onPurchaseMoreStorage();
    void onSmartModeSelected(bool checked);
    void onAdvanceModeSelected(bool checked);

    void onPreferencesValueChanged(QString key);

private:
    Ui::SyncAccountFullMessage* syncAccountFullMessageUI;
    Ui::SyncStallModeSelector* syncStallModeSelectorUI;
};

#endif // SYNCSETTINGSELEMENTS_H
