#ifndef GUESTWINDOW_H
#define GUESTWINDOW_H

#include "qml/QmlDialog.h"

class GuestWindow : public QmlDialog
{
    Q_OBJECT

public:
    explicit GuestWindow(QWindow* parent = nullptr);
    ~GuestWindow() override;

public slots:
    void realocate();

};

#endif // GUESTWINDOW_H
