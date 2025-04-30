#ifndef UPSELL_QML_DIALOG_H
#define UPSELL_QML_DIALOG_H

#include "QmlDialog.h"

class UpsellQmlDialog: public QmlDialog
{
    Q_OBJECT

public:
    using QmlDialog::QmlDialog;
    ~UpsellQmlDialog() override = default;

protected:
    bool event(QEvent*) override;
};

#endif // UPSELL_QML_DIALOG_H
