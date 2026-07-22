#ifndef SCREENSHOT_COMPONENT_H
#define SCREENSHOT_COMPONENT_H

#include "QmlDialogWrapper.h"
#include "ScreenshotAccessController.h"

class ScreenshotComponent: public QMLComponent
{
    Q_OBJECT

public:
    explicit ScreenshotComponent(QObject* parent = nullptr);

    QUrl getQmlUrl() override;

    static void registerQmlModules();

private:
    std::unique_ptr<ScreenshotAccessController> mScreenshotController;
};

#endif
