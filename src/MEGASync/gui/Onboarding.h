#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialog/QmlDialogWrapper.h"
#include "QTMegaRequestListener.h"

#include <QDialog>


class Onboarding : public QMLComponent, public mega::MegaRequestListener
{
    Q_OBJECT

public:

    explicit Onboarding(QObject *parent = 0);
    ~Onboarding();

    QUrl getQmlUrl() override;
    QString contextName() override;

public slots:
    void loginInfo(const QString &email, const QString &password);
};

#endif // ONBOARDING_H
