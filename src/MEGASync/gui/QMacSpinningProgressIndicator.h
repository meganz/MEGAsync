#ifndef QMACSPINNINGPROGRESSINDICATOR_H
#define QMACSPINNINGPROGRESSINDICATOR_H

#include <QPointer>
#include <QWidget>

class QMacSpinningProgressIndicatorPrivate;
class QMacSpinningProgressIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit QMacSpinningProgressIndicator(QWidget *parent = nullptr);
    ~QMacSpinningProgressIndicator();

    qint64 getStartTime() const;

signals:

public slots:
    void animate(bool run = true);
    void start();
    void stop();

private:
    std::unique_ptr<QMacSpinningProgressIndicatorPrivate> pImpl;
    qint64 startTime;

};

#endif // QMACSPINNINGPROGRESSINDICATOR_H
