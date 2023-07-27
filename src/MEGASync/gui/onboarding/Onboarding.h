#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialogWrapper.h"

class Onboarding : public QMLComponent
{
    Q_OBJECT

public:

    explicit Onboarding(QObject *parent = 0);

    QUrl getQmlUrl() override;

    QString contextName() override;

    void showGuestInfoDialog();

    void hideGuestInfoDialog();

    Q_INVOKABLE void openPreferences(bool sync) const;

signals:
    void accountBlocked();
    void logout();

    void showGuestWindow();

    void hideGuestWindow();

};

#endif // ONBOARDING_H
