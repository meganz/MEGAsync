#ifndef UPSELL_COMPONENT_H
#define UPSELL_COMPONENT_H

#include "qml/QmlDialogWrapper.h"
#include "UpsellPlans.h"

#include <memory>

class UpsellController;
class UpsellModel;

class UpsellComponent: public QMLComponent
{
    Q_OBJECT

public:
    explicit UpsellComponent(QObject* parent, UpsellPlans::ViewMode mode);
    virtual ~UpsellComponent() = default;

    QUrl getQmlUrl() override;

    static void registerQmlModules();

    void setTransferFinishTime(long long time);

    UpsellPlans::ViewMode viewMode() const;
    void setViewMode(UpsellPlans::ViewMode mode);

public slots:
    void buyButtonClicked();
    void billedRadioButtonClicked(bool isMonthly);
    void linkInDescriptionClicked();

private:
    std::shared_ptr<UpsellController> mController;
    std::shared_ptr<UpsellModel> mModel;
};

#endif // UPSELL_COMPONENT_H
