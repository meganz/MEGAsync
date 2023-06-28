#ifndef GUESTWINDOW_H
#define GUESTWINDOW_H

#include <QQuickWindow>

class GuestWindow : public QQuickWindow
{
    Q_OBJECT

public:
    explicit GuestWindow(QWindow* parent = nullptr);
    ~GuestWindow() override;

public slots:
    void realocate();

};

#endif // GUESTWINDOW_H
