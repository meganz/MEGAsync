#ifndef GUESTCONTROLLER_H
#define GUESTCONTROLLER_H

#include <QObject>

class GuestController : public QObject
{
    Q_OBJECT

public:

    explicit GuestController(QObject *parent = 0);

    Q_INVOKABLE void onAboutMEGAClicked();
    Q_INVOKABLE void onPreferencesClicked();
    Q_INVOKABLE void onExitClicked();

};

#endif // GUESTCONTROLLER_H
