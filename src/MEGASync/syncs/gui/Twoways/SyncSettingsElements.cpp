#include "SyncSettingsElements.h"
#include "ui_SyncAccountFullMessage.h"

#include <SyncSettingsUIBase.h>
#include <Utilities.h>

SyncSettingsElements::SyncSettingsElements(QObject *parent) :
    QObject(parent),
    syncAccountFullMessageUI(new Ui::SyncAccountFullMessage)
{
}

SyncSettingsElements::~SyncSettingsElements()
{
    delete syncAccountFullMessageUI;
}


void SyncSettingsElements::initElements(SyncSettingsUIBase* syncSettingsUi)
{
    QWidget* syncAccountFull(new QWidget());
    syncAccountFullMessageUI->setupUi(syncAccountFull);
    connect(syncAccountFullMessageUI->bBuyMoreSpace, &QPushButton::clicked, this, &SyncSettingsElements::onPurchaseMoreStorage);

    syncSettingsUi->insertUIElement(syncAccountFull, 1);
}

void SyncSettingsElements::setOverQuotaMode(bool mode)
{
    syncAccountFullMessageUI->wOQError->setVisible(mode);
}

void SyncSettingsElements::onPurchaseMoreStorage()
{
    Utilities::upgradeClicked();
}

