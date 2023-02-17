#ifndef QmlDialog_H
#define QmlDialog_H

#include <QQuickWindow>
//#include <QtQml/qqmlregistration.h>

class QmlDialog : public QQuickWindow
{
    Q_OBJECT

public:
    explicit QmlDialog(QWindow *parent = nullptr);
        ~QmlDialog() override;

signals:
    void finished();

protected:
    bool event(QEvent *) override;

};

#endif // QmlDialog_H
