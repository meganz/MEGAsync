#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "QmlDialogWrapper.h"

class Onboarding: public QMLComponent
{
    Q_OBJECT

public:

    explicit Onboarding(QObject *parent = 0);

    QUrl getQmlUrl() override;

    QString contextName() override;

    Q_INVOKABLE bool deviceNameAlreadyExists(const QString& name) const;
    Q_INVOKABLE void openDeviceCentre() const;

signals:
    void accountBlocked(int errorCode);
    void logout();
};

#endif // ONBOARDING_H
