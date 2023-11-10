#ifndef ONBOARDINGQMLDIALOG_H
#define ONBOARDINGQMLDIALOG_H

#include "qml/QmlDialog.h"

class OnboardingQmlDialog : public QmlDialog
{
    Q_OBJECT

    Q_PROPERTY(bool loggingIn MEMBER mLoggingIn READ getLoggingIn WRITE setLoggingIn NOTIFY loggingInChanged)
    Q_PROPERTY(bool creatingAccount MEMBER mCreatingAccount READ getCreatingAccount WRITE setCreatingAccount NOTIFY creatingAccountChanged)

public:
    explicit OnboardingQmlDialog(QWindow *parent = nullptr);
    ~OnboardingQmlDialog() override;

    bool getCreatingAccount() const;
    void setCreatingAccount(bool value);
    bool getLoggingIn() const;
    void setLoggingIn(bool value);
    Q_INVOKABLE void forceClose();
    Q_INVOKABLE void raise();

signals:
    void loggingInChanged();
    void creatingAccountChanged();
    void closingButLoggingIn();
    void closingButCreatingAccount();

protected:
    bool event(QEvent *) override;

private:
    bool mLoggingIn;
    bool mCloseClicked;
    bool mForceClose;
    bool mCreatingAccount;
};

#endif // ONBOARDINGQMLDIALOG_H
