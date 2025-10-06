#ifndef CHANGE_PASSWORD_COMPONENT_H
#define CHANGE_PASSWORD_COMPONENT_H

#include "QmlDialogWrapper.h"

class ChangePasswordComponent: public QMLComponent
{
    Q_OBJECT

public:
    explicit ChangePasswordComponent(QObject* parent = 0);

    QUrl getQmlUrl() override;

    static void registerQmlModules();

    Q_INVOKABLE void changePassword(QString pass);

signals:
    void passwordChanged();

private:
};

#endif
