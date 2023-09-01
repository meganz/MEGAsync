#ifndef ONBOARDINGQMLDIALOG_H
#define ONBOARDINGQMLDIALOG_H

#include "qml/QmlDialog.h"

class OnboardingQmlDialog : public QmlDialog
{
    Q_OBJECT

    Q_PROPERTY(bool loggingIn MEMBER mLoggingIn READ getLoggingIn WRITE setLoggingIn NOTIFY loggingInChanged)

public:
    explicit OnboardingQmlDialog(QWindow *parent = nullptr);
    ~OnboardingQmlDialog() override;

    Q_INVOKABLE bool getLoggingIn() const;
    Q_INVOKABLE void setLoggingIn(bool value);
    Q_INVOKABLE void forceClose();

signals:
    void loggingInChanged();
    void closingButLoggingIn();
    void closingButCreatingAccount();

protected:
    bool event(QEvent *) override;

private:
    bool mLoggingIn;
    bool mCloseClicked;
    bool mForceClose;
};

#endif // ONBOARDINGQMLDIALOG_H
