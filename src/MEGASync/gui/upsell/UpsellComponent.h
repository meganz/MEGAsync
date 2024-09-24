#ifndef UPSELL_COMPONENT_H
#define UPSELL_COMPONENT_H

#include "qml/QmlDialogWrapper.h"

#include <memory>

class UpsellController;
class UpsellModel;

class UpsellComponent: public QMLComponent
{
    Q_OBJECT

public:
    explicit UpsellComponent(QObject* parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    static void registerQmlModules();

public slots:
    void buyButtonClicked();

private:
    std::shared_ptr<UpsellController> mController;
    std::shared_ptr<UpsellModel> mModel;
};

#endif // UPSELL_COMPONENT_H
