#ifndef GUESTQMLDIALOG_H
#define GUESTQMLDIALOG_H

#include "qml/QmlDialog.h"

class GuestQmlDialog : public QmlDialog
{
    Q_OBJECT

public:
    explicit GuestQmlDialog(QWindow* parent = nullptr);
    ~GuestQmlDialog() override;

public slots:
    void realocate();

};

#endif // GUESTQMLDIALOG_H
