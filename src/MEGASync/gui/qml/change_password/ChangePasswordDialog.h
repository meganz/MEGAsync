#ifndef CHANGE_PASSWORD_DIALOG_H
#define CHANGE_PASSWORD_DIALOG_H

#include "QmlDialog.h"

class ChangePasswordDialog: public QmlDialog
{
    Q_OBJECT

public:
    using QmlDialog::QmlDialog;
    ~ChangePasswordDialog() override = default;

protected:
    bool event(QEvent*) override;
};

#endif
