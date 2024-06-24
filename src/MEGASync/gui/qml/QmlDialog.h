#ifndef QMLDIALOG_H
#define QMLDIALOG_H

#include <QQuickWindow>

class QmlDialog : public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(QString iconSrc MEMBER mIconSrc WRITE setIconSrc)

public:
    explicit QmlDialog(QWindow* parent = nullptr);
    ~QmlDialog() = default;

    Q_INVOKABLE void setIconSrc(const QString& iconSrc);

signals:
    void accept();
    void reject();
    void finished();
    void languageChanged();
    void accepted();
    void rejected();
    void requestPageFocus();
    void initializePageFocus();

protected:
    bool event(QEvent*) override;
    void onRequestPageFocus();

private:
    QString mIconSrc;

};

#endif // QMLDIALOG_H
