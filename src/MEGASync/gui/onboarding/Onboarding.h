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

    Q_INVOKABLE void openPreferences(int tabIndex) const;
    Q_INVOKABLE bool deviceNameAlreadyExists(const QString& name) const;

signals:
    void accountBlocked(int errorCode);
    void logout();
};

#endif // ONBOARDING_H
