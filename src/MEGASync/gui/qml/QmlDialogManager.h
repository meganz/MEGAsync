#ifndef QML_DIALOG_MANAGER_H
#define QML_DIALOG_MANAGER_H

#include <memory>
#include <QQmlComponent>
#include <QTimer>

class QmlDialogManager: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static QmlDialogManager* getQmlInstance(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static std::shared_ptr<QmlDialogManager> instance();

    QmlDialogManager(const QmlDialogManager&) = delete;
    QmlDialogManager& operator=(const QmlDialogManager&) = delete;

    void openGuestDialog();
    bool openOnboardingDialog();
    bool openWhatsNewDialog();

    bool raiseGuestDialog();
    void raiseOnboardingDialog();
    void raiseOrHideInfoGuestDialog(QTimer* dialogTimer, int msec);
    void forceCloseOnboardingDialog();

private:
    explicit QmlDialogManager(QObject* parent = nullptr) {}
};

#endif // QML_DIALOG_MANAGER_H
