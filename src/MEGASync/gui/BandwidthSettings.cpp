#include "BandwidthSettings.h"

#include "ui_BandwidthSettings.h"

#include <QButtonGroup>

BandwidthSettings::BandwidthSettings(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::BandwidthSettings),
    mApp(app),
    mPreferences(Preferences::instance())
{
    mUi->setupUi(this);

    mUi->eUploadLimit->setValidator(new QIntValidator(0, 1000000000, this));
    mUi->eDownloadLimit->setValidator(new QIntValidator(0, 1000000000, this));

    mUi->eMaxDownloadConnections->setRange(1, BandwidthSettings::MAX_NUM_CONNECTIONS);
    mUi->eMaxUploadConnections->setRange(1, BandwidthSettings::MAX_NUM_CONNECTIONS);

    QButtonGroup* downloadButtonGroup = new QButtonGroup(this);
    downloadButtonGroup->addButton(mUi->rDownloadLimit);
    downloadButtonGroup->addButton(mUi->rDownloadNoLimit);
    QButtonGroup* uploadButtonGroup = new QButtonGroup(this);
    uploadButtonGroup->addButton(mUi->rUploadLimit);
    uploadButtonGroup->addButton(mUi->rUploadNoLimit);
    uploadButtonGroup->addButton(mUi->rUploadAutoLimit);

    initialize();
}

BandwidthSettings::~BandwidthSettings()
{
    delete mUi;
}

void BandwidthSettings::initialize()
{
    int uploadLimitKB = mPreferences->uploadLimitKB();
    mUi->rUploadAutoLimit->setChecked(uploadLimitKB == BANDWIDTH_LIMIT_AUTO);
    mUi->rUploadLimit->setChecked(uploadLimitKB > 0);
    mUi->rUploadNoLimit->setChecked(uploadLimitKB == BANDWIDTH_LIMIT_NONE);
    mUi->eUploadLimit->setText((uploadLimitKB <= 0) ?
                                   QString::fromUtf8("0")
                                 : QString::number(uploadLimitKB));
    mUi->eUploadLimit->setEnabled(mUi->rUploadLimit->isChecked());

    int downloadLimitKB = mPreferences->downloadLimitKB();
    mUi->rDownloadLimit->setChecked(downloadLimitKB > 0);
    mUi->rDownloadNoLimit->setChecked(downloadLimitKB == BANDWIDTH_LIMIT_NONE);
    mUi->eDownloadLimit->setText((downloadLimitKB <= 0) ?
                                     QString::fromUtf8("0")
                                   : QString::number(downloadLimitKB));
    mUi->eDownloadLimit->setEnabled(mUi->rDownloadLimit->isChecked());

    mUi->eMaxDownloadConnections->setValue(mPreferences->parallelDownloadConnections());
    mUi->eMaxUploadConnections->setValue(mPreferences->parallelUploadConnections());

    mUi->cbUseHttps->setChecked(mPreferences->usingHttpsOnly());
}

void BandwidthSettings::on_rUploadAutoLimit_toggled(bool checked)
{
    mUi->eUploadLimit->setEnabled(!checked);
}

void BandwidthSettings::on_rUploadNoLimit_toggled(bool checked)
{
    mUi->eUploadLimit->setEnabled(!checked);
}

void BandwidthSettings::on_rUploadLimit_toggled(bool checked)
{
    mUi->eUploadLimit->setEnabled(checked);
}

void BandwidthSettings::on_rDownloadNoLimit_toggled(bool checked)
{
    mUi->eDownloadLimit->setEnabled(!checked);
}

void BandwidthSettings::on_rDownloadLimit_toggled(bool checked)
{
    mUi->eDownloadLimit->setEnabled(checked);
}

void BandwidthSettings::on_bUpdate_clicked()
{
    if (mUi->rUploadNoLimit->isChecked())
    {
        mPreferences->setUploadLimitKB(BANDWIDTH_LIMIT_NONE);
    }
    else if (mUi->rUploadAutoLimit->isChecked())
    {
        mPreferences->setUploadLimitKB(BANDWIDTH_LIMIT_AUTO);
    }
    else
    {
        mPreferences->setUploadLimitKB(mUi->eUploadLimit->text().toInt());
    }

    if (mUi->rDownloadNoLimit->isChecked())
    {
        mPreferences->setDownloadLimitKB(BANDWIDTH_LIMIT_NONE);
    }
    else
    {
        mPreferences->setDownloadLimitKB(mUi->eDownloadLimit->text().toInt());
    }

    if (mUi->eMaxUploadConnections->value() != mPreferences->parallelUploadConnections())
    {
        mPreferences->setParallelUploadConnections(mUi->eMaxUploadConnections->value());
    }

    if (mUi->eMaxDownloadConnections->value() != mPreferences->parallelDownloadConnections())
    {
        mPreferences->setParallelDownloadConnections(mUi->eMaxDownloadConnections->value());
    }

    if (mUi->cbUseHttps->isChecked() != mPreferences->usingHttpsOnly())
    {
        mPreferences->setUseHttpsOnly(mUi->cbUseHttps->isChecked());
    }

    accept();
}

void BandwidthSettings::on_bCancel_clicked()
{
    reject();
}
