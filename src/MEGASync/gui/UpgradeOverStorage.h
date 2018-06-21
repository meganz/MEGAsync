#ifndef UPGRADEOVERSTORAGE_H
#define UPGRADEOVERSTORAGE_H

#include <QDialog>
#include "gui/UpgradeWidget.h"
#include <QHBoxLayout>
#include <megaapi.h>

namespace Ui {
class UpgradeOverStorage;
}

class UpgradeOverStorage : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeOverStorage(mega::MegaApi *megaApi, mega::MegaPricing *pricing, QWidget *parent = 0);
    void refreshUsedStorage();
    void setPricing(mega::MegaPricing *pricing);
    ~UpgradeOverStorage();

private:
    Ui::UpgradeOverStorage *ui;
    QHBoxLayout* plansLayout;

    void updatePlans();
    void checkAchievementsEnabled();
    QString convertCurrency(const char *currency);
    void clearPlans();

protected:
    void changeEvent(QEvent* event);

    mega::MegaPricing *pricing;
    mega::MegaApi *megaApi;
};

#endif // UPGRADEOVERSTORAGE_H
