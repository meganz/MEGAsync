#ifndef SYNCSQMLDIALOG_H
#define SYNCSQMLDIALOG_H

#include "QmlDialog.h"

class SyncsQmlDialog : public QmlDialog
{
    Q_OBJECT

public:
    using QmlDialog::QmlDialog;
    ~SyncsQmlDialog() override = default;

protected:
    bool event(QEvent*) override;

};

#endif // SYNCSQMLDIALOG_H
