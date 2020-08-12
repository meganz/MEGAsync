#ifndef QSPINNINGPROGRESSINDICATOR_H
#define QSPINNINGPROGRESSINDICATOR_H

#include <QPointer>
#include <QWidget>

class QSpinningProgressIndicatorPrivate;
class QSpinningProgressIndicator : public QWidget
{
    Q_OBJECT
public:
    explicit QSpinningProgressIndicator(QWidget *parent = nullptr);
    ~QSpinningProgressIndicator();

    qint64 getStartTime() const;

signals:

public slots:
    void animate(bool run = true);

private:
    std::unique_ptr<QSpinningProgressIndicatorPrivate> pImpl;
    qint64 startTime;

};

#endif // QSPINNINGPROGRESSINDICATOR_H
