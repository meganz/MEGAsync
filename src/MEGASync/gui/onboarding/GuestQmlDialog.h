#ifndef GUESTQMLDIALOG_H
#define GUESTQMLDIALOG_H

#include "qml/QmlDialog.h"

class GuestQmlDialog : public QmlDialog
{
    Q_OBJECT

public:
    explicit GuestQmlDialog(QWindow* parent = nullptr);
    ~GuestQmlDialog() override;
    bool isHiddenForLongTime() const;

signals:
    void guestActiveChanged(bool active);

public slots:
    void realocate();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    qint64 mLastHideTime = 0;
};

#endif // GUESTQMLDIALOG_H
