#ifndef QMLDIALOG_H
#define QMLDIALOG_H

#include <QQuickWindow>

class QmlDialog : public QQuickWindow
{
    Q_OBJECT

public:
    explicit QmlDialog(QWindow* parent = nullptr);
    ~QmlDialog() = default;

signals:
    void finished();
    void languageChanged();
    void accepted();
    void rejected();
    void requestPageFocus();
    void initializePageFocus();

protected:
    bool event(QEvent*) override;
    void onRequestPageFocus();

};

#endif // QMLDIALOG_H
