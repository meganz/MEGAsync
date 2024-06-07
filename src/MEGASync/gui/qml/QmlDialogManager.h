#ifndef QMLDIALOGMANAGER_H
#define QMLDIALOGMANAGER_H

#include <QTimer>

#include <memory>

class QmlDialogManager
{
public:
    static std::shared_ptr<QmlDialogManager> instance();

    QmlDialogManager(const QmlDialogManager&) = delete;
    QmlDialogManager& operator=(const QmlDialogManager&) = delete;

    void openGuestDialog();
    bool openOnboardingDialog();
    bool openWhatsNewDialog();
    void openBackupsDialog(bool fromSettings = false);

    bool raiseGuestDialog();
    void raiseOnboardingDialog();

    void raiseOrHideInfoGuestDialog(QTimer* dialogTimer, int msec);

    void forceCloseOnboardingDialog();

private:
    QmlDialogManager();

};

#endif // QMLDIALOGMANAGER_H
