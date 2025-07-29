#ifndef BANDWIDTHSETTINGS_H
#define BANDWIDTHSETTINGS_H

#include "MegaApplication.h"
#include "Preferences.h"

#include <QDialog>

namespace Ui {
class BandwidthSettings;
}

class BandwidthSettings : public QDialog
{
    Q_OBJECT

public:
    explicit BandwidthSettings(MegaApplication* app, QWidget* parent = nullptr);
    ~BandwidthSettings();

    enum SettingChanged
    {
        NONE = 0x0,
        UPLOAD_LIMIT = 0x1,
        DOWNLOAD_LIMIT = 0x02,
        UPLOAD_CONNECTIONS = 0X04,
        DOWNLOAD_CONNECTIONS = 0x08,
        USE_HTTPS = 0x10
    };

    Q_DECLARE_FLAGS(SettingsChanged, SettingChanged);

    bool settingHasChanged(SettingChanged setting) const;

private slots:
    void on_rUploadAutoLimit_toggled(bool checked);
    void on_rUploadNoLimit_toggled(bool checked);
    void on_rUploadLimit_toggled(bool checked);
    void on_rDownloadNoLimit_toggled(bool checked);
    void on_rDownloadLimit_toggled(bool checked);

    void on_bUpdate_clicked();
    void on_bCancel_clicked();

private:
    enum
    {
        BANDWIDTH_LIMIT_AUTO = -1,
        BANDWIDTH_LIMIT_NONE = 0,
    };
    void initialize();

    Ui::BandwidthSettings* mUi;
    MegaApplication* mApp;
    std::shared_ptr<Preferences> mPreferences;
    SettingsChanged mSettingsChanged;
};

#endif // BANDWIDTHSETTINGS_H
