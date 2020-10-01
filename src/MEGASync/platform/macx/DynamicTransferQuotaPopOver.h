#ifndef DYNAMICTRANSFERQUOTAPOPOVER_H
#define DYNAMICTRANSFERQUOTAPOPOVER_H

#include <QMacNativeWidget>
#include "ui_DynamicTransferQuotaPopOver.h"

class DynamicTransferQuotaPopOver : public QMacNativeWidget
{
    Q_OBJECT
public:
    DynamicTransferQuotaPopOver();
    void updateMessage(const QString& message);

protected:
    void changeEvent(QEvent *event);
    void tweakStrings();

private:
    Ui::DynamicTransferQuotaPopOver m_ui;
};

#endif // DYNAMICTRANSFERQUOTAPOPOVER_H
