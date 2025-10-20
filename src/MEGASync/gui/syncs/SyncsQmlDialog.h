#ifndef SYNCS_QML_DIALOG_H
#define SYNCS_QML_DIALOG_H

#include "QmlDialog.h"

class SyncsQmlDialog: public QmlDialog
{
    Q_OBJECT

public:
    Q_PROPERTY(bool backup READ isBackup WRITE setIsBackup)

    using QmlDialog::QmlDialog;
    ~SyncsQmlDialog() override = default;

    bool isBackup() const;
    void setIsBackup(bool newIsBackup);

protected:
    bool event(QEvent*) override;

private:
    bool mIsBackup = false;
};

#endif // SYNCS_QML_DIALOG_H
