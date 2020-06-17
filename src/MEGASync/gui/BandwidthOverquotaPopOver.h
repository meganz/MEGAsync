#pragma once

#include <QDialog>
#include "ui_BandwidthOverquotaPopOver.h"

namespace Ui {
class BandwidthOverquotaPopOver;
}

class BandwidthOverquotaPopOver : public QDialog
{
    Q_OBJECT

public:
    explicit BandwidthOverquotaPopOver(QDialog *parent = 0);
    BandwidthOverquotaPopOver();
    void updateMessage(const QString& message);

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent* event);

    void tweakStrings();

private:
    Ui::BandwidthOverquotaPopOver *ui;
};


