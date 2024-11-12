#ifndef WHATS_NEW_WINDOW_H
#define WHATS_NEW_WINDOW_H

#include "QmlDialogWrapper.h"
#include "UpdatesModel.h"
#include "WhatsNewController.h"

class WhatsNewWindow: public QMLComponent
{
    Q_OBJECT

public:
    explicit WhatsNewWindow(QObject* parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    Q_INVOKABLE QString acceptButtonText();
    Q_INVOKABLE void acceptButtonClicked();

private:
    std::shared_ptr<WhatsNewController> mController;
    std::shared_ptr<UpdatesModel> mModel;
};
#endif // WHATS_NEW_WINDOW_H
