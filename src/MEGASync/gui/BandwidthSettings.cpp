#include "BandwidthSettings.h"

#include "ParallelConnectionsValues.h"
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

bool BandwidthSettings::settingHasChanged(SettingChanged setting) const
{
    return mSettingsChanged.testFlag(setting);
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

    mUi->eMaxDownloadConnections->setRange(ParallelConnectionsValues::getMinValue(),
                                           ParallelConnectionsValues::getMaxValue());
    mUi->eMaxUploadConnections->setRange(ParallelConnectionsValues::getMinValue(),
                                         ParallelConnectionsValues::getMaxValue());
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
    // Reset Flags
    mSettingsChanged = SettingChanged::NONE;

    // UPLOAD
    auto currentUploadLimit(mPreferences->uploadLimitKB());

    if (mUi->rUploadNoLimit->isChecked())
    {
        if (currentUploadLimit != BANDWIDTH_LIMIT_NONE)
        {
            mSettingsChanged.setFlag(UPLOAD_LIMIT, true);

            mPreferences->setUploadLimitKB(BANDWIDTH_LIMIT_NONE);
        }
    }
    else if (mUi->rUploadAutoLimit->isChecked())
    {
        if (currentUploadLimit != BANDWIDTH_LIMIT_AUTO)
        {
            mSettingsChanged.setFlag(UPLOAD_LIMIT, true);

            mPreferences->setUploadLimitKB(BANDWIDTH_LIMIT_AUTO);
        }
    }
    else
    {
        auto newUploadLimit(mUi->eUploadLimit->text().toInt());
        if (newUploadLimit != currentUploadLimit)
        {
            mSettingsChanged.setFlag(UPLOAD_LIMIT, true);

            mPreferences->setUploadLimitKB(newUploadLimit);
        }
    }

    // DOWNLOAD
    auto currentDownloadLimit(mPreferences->downloadLimitKB());

    if (mUi->rDownloadNoLimit->isChecked())
    {
        if (currentDownloadLimit != BANDWIDTH_LIMIT_NONE)
        {
            mSettingsChanged.setFlag(DOWNLOAD_LIMIT, true);

            mPreferences->setDownloadLimitKB(BANDWIDTH_LIMIT_NONE);
        }
    }
    else
    {
        auto newDownloadLimit(mUi->eDownloadLimit->text().toInt());

        if (newDownloadLimit != currentDownloadLimit)
        {
            mSettingsChanged.setFlag(DOWNLOAD_LIMIT, true);

            mPreferences->setDownloadLimitKB(newDownloadLimit);
        }
    }

    // MAX CONNECTIONS
    if (mUi->eMaxUploadConnections->value() != mPreferences->parallelUploadConnections())
    {
        mSettingsChanged.setFlag(UPLOAD_CONNECTIONS, true);

        mPreferences->setParallelUploadConnections(mUi->eMaxUploadConnections->value());
    }

    if (mUi->eMaxDownloadConnections->value() != mPreferences->parallelDownloadConnections())
    {
        mSettingsChanged.setFlag(DOWNLOAD_CONNECTIONS, true);

        mPreferences->setParallelDownloadConnections(mUi->eMaxDownloadConnections->value());
    }

    // URL
    if (mUi->cbUseHttps->isChecked() != mPreferences->usingHttpsOnly())
    {
        mSettingsChanged.setFlag(USE_HTTPS, true);

        mPreferences->setUseHttpsOnly(mUi->cbUseHttps->isChecked());
    }

    accept();
}

void BandwidthSettings::on_bCancel_clicked()
{
    reject();
}
