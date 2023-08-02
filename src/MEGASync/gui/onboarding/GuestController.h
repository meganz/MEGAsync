#ifndef GUESTCONTROLLER_H
#define GUESTCONTROLLER_H

#include "qml/QmlDialogWrapper.h"

class GuestController : public QMLComponent
{
    Q_OBJECT

public:

    explicit GuestController(QObject *parent = 0);

    QUrl getQmlUrl() override;

    QString contextName() override;

    Q_INVOKABLE void onAboutMEGAClicked();
    Q_INVOKABLE void onPreferencesClicked();
    Q_INVOKABLE void onExitClicked();

};

#endif // GUESTCONTROLLER_H
