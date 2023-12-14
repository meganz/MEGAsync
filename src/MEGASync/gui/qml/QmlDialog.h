#ifndef QmlDialog_H
#define QmlDialog_H

#include <QQuickWindow>

class QmlDialog : public QQuickWindow
{
    Q_OBJECT

public:
    explicit QmlDialog(QWindow* parent = nullptr);

signals:
    void finished();

protected:
    bool event(QEvent* ) override;

};

#endif // QmlDialog_H
