#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include <QDialog>
#include <QTimer>
#include <megaapi.h>

namespace Ui {
class UpgradeDialog;
}

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(mega::MegaApi* api, QWidget *parent = 0);
    void setTimestamp(long long finishTime);
    void refreshAccountDetails();
    ~UpgradeDialog();

private:
    Ui::UpgradeDialog *ui;
    long long finishTime;
    QTimer *timer;

private slots:
    void unitTimeElapsed();

protected:
    mega::MegaApi *megaApi;
    mega::MegaPricing *pricing;
};

#endif // UPGRADEDIALOG_H
