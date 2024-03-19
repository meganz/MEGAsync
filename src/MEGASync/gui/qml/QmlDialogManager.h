#ifndef QMLDIALOGMANAGER_H
#define QMLDIALOGMANAGER_H

#include <QObject>

#include <memory>

class QmlDialogManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<QmlDialogManager> instance();

    QmlDialogManager(const QmlDialogManager&) = delete;
    QmlDialogManager& operator=(const QmlDialogManager&) = delete;

    void openGuestDialog();
    void openOnboardingDialog();

    bool raiseGuestDialog();
    void raiseOnboardingDialog();

    void raiseOrHideInfoGuestDialog(QTimer* dialogTimer, int msec);

    void forceCloseOnboardingDialog();

private:
    QmlDialogManager();

};

#endif // QMLDIALOGMANAGER_H
