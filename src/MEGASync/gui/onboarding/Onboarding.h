#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "QmlDialogWrapper.h"

class SyncsComponent;
class Onboarding: public QMLComponent
{
    Q_OBJECT

public:
    explicit Onboarding(QObject* parent = 0);
    QUrl getQmlUrl() override;

    Q_INVOKABLE void openPreferences(int tabIndex) const;
    Q_INVOKABLE void checkDeviceName(const QString& name);
    Q_INVOKABLE void showClosingButLoggingInWarningDialog() const;
    Q_INVOKABLE void showClosingButCreatingAccount() const;

signals:
    void accountBlocked(int errorCode);
    void logout();
    void deviceNameChecked(bool valid);

private:
    std::unique_ptr<SyncsComponent> mSyncsComponent;
};

#endif // ONBOARDING_H
