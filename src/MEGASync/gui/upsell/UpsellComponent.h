#ifndef UPSELL_COMPONENT_H
#define UPSELL_COMPONENT_H

#include "qml/QmlDialogWrapper.h"

class UpsellComponent: public QMLComponent
{
    Q_OBJECT

public:
    explicit UpsellComponent(QObject* parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    static void registerQmlModules();
};

#endif // UPSELL_COMPONENT_H
