#include "QSpinningProgressIndicator.h"

#include <QDateTime>
#include <QProgressBar>
#include <QHBoxLayout>


class QSpinningProgressIndicatorPrivate : public QObject
{
public:
    QSpinningProgressIndicatorPrivate(QSpinningProgressIndicator *qProgressIndicatorSpinning,
                                      QProgressBar *nsProgressIndicator)
        : QObject(qProgressIndicatorSpinning), spinningProgressindicator(nsProgressIndicator) {}

    QPointer<QProgressBar> spinningProgressindicator;
};

QSpinningProgressIndicator::QSpinningProgressIndicator(QWidget *parent)
    : QWidget(parent), startTime(0)
{
    std::unique_ptr<QProgressBar> progress {new QProgressBar(this)};
    progress->setRange(0,0);
    pImpl.reset(new QSpinningProgressIndicatorPrivate(this, progress.get()));

    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setMargin(0);
    layout->addWidget(progress.get());
}

QSpinningProgressIndicator::~QSpinningProgressIndicator()
{
}

void QSpinningProgressIndicator::animate(bool animate)
{
    Q_ASSERT(pImpl);

    if (!pImpl)
    {
        return;
    }

    if (animate)
    {
        startTime = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        startTime = 0;
    }

    //Fix me: Infinite progress bar does not have way to stop, start.
    //Check if we need this or remove
    pImpl->spinningProgressindicator->setValue(0);
}

qint64 QSpinningProgressIndicator::getStartTime() const
{
    return startTime;
}
