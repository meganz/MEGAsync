#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "QmlDialogWrapper.h"
#include "SyncsComponent.h"

#include <QObject>
#include <QUrl>

#include <memory>

class Onboarding: public QMLComponent
{
    Q_OBJECT

public:
    explicit Onboarding(QObject* parent = 0);

    QUrl getQmlUrl() override;

    Q_INVOKABLE void openPreferences(int tabIndex) const;
    Q_INVOKABLE void openMigrationTool() const;
    Q_INVOKABLE void showClosingButLoggingInWarningDialog() const;
    Q_INVOKABLE void showClosingButCreatingAccount() const;

signals:
    void accountBlocked(int errorCode);
    void logout();

private:
    std::unique_ptr<SyncsComponent> mSyncsComponent;
};

#endif // ONBOARDING_H
