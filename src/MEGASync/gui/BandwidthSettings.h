#ifndef BANDWIDTHSETTINGS_H
#define BANDWIDTHSETTINGS_H

#include <QDialog>

#include "MegaApplication.h"
#include "Preferences.h"

namespace Ui {
class BandwidthSettings;
}

class BandwidthSettings : public QDialog
{
    Q_OBJECT

public:       
    explicit BandwidthSettings(MegaApplication* app, QWidget* parent = nullptr);
    ~BandwidthSettings();

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
};

#endif // BANDWIDTHSETTINGS_H
