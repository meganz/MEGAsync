#ifndef QmlDialog_H
#define QmlDialog_H

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

protected:
    bool event(QEvent* ) override;
};

#endif // QmlDialog_H
