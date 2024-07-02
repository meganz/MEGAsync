#ifndef EXCLUSIONSQMLDIALOG_H
#define EXCLUSIONSQMLDIALOG_H

#include "QmlDialog.h"
#include "syncs/control/MegaIgnoreManager.h"
#include "ExclusionRulesModel.h"

class ExclusionsQmlDialog : public QmlDialog
{
public:

    explicit ExclusionsQmlDialog(QWindow *parent = nullptr);
    ~ExclusionsQmlDialog();
};

#endif // EXCLUSIONSQMLDIALOG_H
