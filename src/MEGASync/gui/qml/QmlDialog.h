#ifndef QML_DIALOG_H
#define QML_DIALOG_H

#include "QmlInstancesManager.h"

#include <QQuickWindow>

class QmlDialog: public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(QString iconSrc MEMBER mIconSrc WRITE setIconSrc)
    Q_PROPERTY(bool closeOnEscapePressed READ getCloseOnEscapePressed WRITE setCloseOnEscapePressed)
    Q_PROPERTY(QmlInstancesManager* instancesManager READ getInstancesManager NOTIFY
                   instancesManagerChanged)

public:
    explicit QmlDialog(QWindow* parent = nullptr);
    ~QmlDialog() = default;

public slots:
    void setIconSrc(const QString& iconSrc);
    QmlInstancesManager* getInstancesManager();
    void centerAndRaise();
    bool getCloseOnEscapePressed() const;
    void setCloseOnEscapePressed(bool active);

signals:
    void instancesManagerChanged();
    void accept();
    void reject();
    void finished();
    void accepted();
    void rejected();
    void requestPageFocus();
    void initializePageFocus();
    void closeOnEscapePressedChanged();

protected:
    bool event(QEvent* event) override;
    void onRequestPageFocus();

private:
    QString mIconSrc;
    bool mCloseOnEscapePressed = false;
    QPointer<QmlInstancesManager> mInstancesManager;
};

#endif // QML_DIALOG_H
