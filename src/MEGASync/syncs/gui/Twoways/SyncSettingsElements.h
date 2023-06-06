#ifndef SYNCSETTINGSELEMENTS_H
#define SYNCSETTINGSELEMENTS_H

#include <QWidget>

namespace Ui {
class SyncAccountFullMessage;
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

private:
    Ui::SyncAccountFullMessage* syncAccountFullMessageUI;
};

#endif // SYNCSETTINGSELEMENTS_H
