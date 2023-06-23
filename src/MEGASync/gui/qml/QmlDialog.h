#ifndef QmlDialog_H
#define QmlDialog_H

#include <QQuickWindow>

class QmlDialog : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(bool loggingIn MEMBER mLoggingIn READ getLoggingIn WRITE setLoggingIn NOTIFY loggingInChanged)

public:
    explicit QmlDialog(QWindow *parent = nullptr);
    ~QmlDialog() override;

    Q_INVOKABLE bool getLoggingIn() const;
    Q_INVOKABLE void setLoggingIn(bool value);

signals:
    void finished();
    void loggingInChanged();
protected:
    bool event(QEvent *) override;

private:
    bool mLoggingIn;
    bool mCloseClicked;
};

#endif // QmlDialog_H
