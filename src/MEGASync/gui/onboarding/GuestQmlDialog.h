#ifndef GUESTQMLDIALOG_H
#define GUESTQMLDIALOG_H

#include "qml/QmlDialog.h"
#include "qtimer.h"

class GuestQmlDialog : public QmlDialog
{
    Q_OBJECT

public:
    explicit GuestQmlDialog(QWindow* parent = nullptr);
    ~GuestQmlDialog() override;

public slots:
    void realocate();

protected:
    void showEvent(QShowEvent* ) override;

private:
    QTimer mHideTimer;
};

#endif // GUESTQMLDIALOG_H
