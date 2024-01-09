#ifndef BACKUPSQMLDIALOG_H
#define BACKUPSQMLDIALOG_H

#include "qml/QmlDialog.h"

class BackupsQmlDialog : public QmlDialog
{
    Q_OBJECT

public:
    explicit BackupsQmlDialog(QWindow *parent = nullptr);
    ~BackupsQmlDialog() override = default;

};

#endif // BACKUPSQMLDIALOG_H
