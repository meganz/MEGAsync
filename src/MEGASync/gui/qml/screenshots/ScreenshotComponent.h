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

    // Called from QML when the user accepts the one-time enable offer.
    Q_INVOKABLE void enableScreenshotUploads();

    static void registerQmlModules();

private:
    std::unique_ptr<ScreenshotAccessController> mScreenshotController;
};

#endif
