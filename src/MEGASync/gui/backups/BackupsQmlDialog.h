#ifndef BACKUPSQMLDIALOG_H
#define BACKUPSQMLDIALOG_H

#include "QmlDialog.h"

class BackupsQmlDialog: public QmlDialog
{
    Q_OBJECT

public:
    using QmlDialog::QmlDialog;
    ~BackupsQmlDialog() override = default;

protected:
    bool event(QEvent*) override;
};

#endif // BACKUPSQMLDIALOG_H
