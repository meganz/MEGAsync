#ifndef EXCLUSIONSQMLDIALOG_H
#define EXCLUSIONSQMLDIALOG_H

#include "qml/QmlDialog.h"
#include "syncs/control/MegaIgnoreManager.h"
#include "gui/SyncExclusions/ExclusionRulesModel.h"

class ExclusionsQmlDialog : public QmlDialog
{
public:

    explicit ExclusionsQmlDialog(QWindow *parent = nullptr);
    ~ExclusionsQmlDialog();
};

#endif // EXCLUSIONSQMLDIALOG_H
