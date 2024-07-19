#ifndef QML_DIALOG_MANAGER_H
#define QML_DIALOG_MANAGER_H

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
    void openAddSync(const QString& remoteFolder, bool fromSettings);

    bool raiseGuestDialog();
    void raiseOnboardingDialog();
    void raiseOrHideInfoGuestDialog(QTimer* dialogTimer, int msec);
    void forceCloseOnboardingDialog();

private:
    QmlDialogManager() = default;
};

#endif // QML_DIALOG_MANAGER_H
