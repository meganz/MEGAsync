#pragma once

#include <QDialog>
#include "ui_DynamicTransferQuotaPopOver.h"

namespace Ui {
class DynamicTransferQuotaPopOver;
}

class DynamicTransferQuotaPopOver : public QDialog
{
    Q_OBJECT

public:
    explicit DynamicTransferQuotaPopOver(QDialog *parent = 0);
    DynamicTransferQuotaPopOver();
    void updateMessage(const QString& message);

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent* event);

    void tweakStrings();

private:
    Ui::DynamicTransferQuotaPopOver *ui;
};


