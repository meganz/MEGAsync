#include "SyncSettings.h"
#include "ui_SyncSettings.h"

SyncSettings::SyncSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SyncSettings)
{
    ui->setupUi(this);
}

SyncSettings::~SyncSettings()
{
    delete ui;
}
