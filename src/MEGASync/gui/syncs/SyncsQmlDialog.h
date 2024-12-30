#ifndef SYNCS_QML_DIALOG_H
#define SYNCS_QML_DIALOG_H

#include "QmlDialog.h"

class SyncsQmlDialog: public QmlDialog
{
    Q_OBJECT

public:
    using QmlDialog::QmlDialog;
    ~SyncsQmlDialog() override = default;

protected:
    bool event(QEvent*) override;
};

#endif // SYNCS_QML_DIALOG_H
