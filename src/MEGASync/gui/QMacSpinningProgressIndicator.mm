#include "QMacSpinningProgressIndicator.h"

#import "Foundation/NSAutoreleasePool.h"
#import "AppKit/NSProgressIndicator.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QMacCocoaViewContainer>


class QMacSpinningProgressIndicatorPrivate : public QObject
{
public:
    QMacSpinningProgressIndicatorPrivate(QMacSpinningProgressIndicator *qProgressIndicatorSpinning,
                                      NSProgressIndicator *nsProgressIndicator)
        : QObject(qProgressIndicatorSpinning), nsProgressIndicator(nsProgressIndicator) {}


    ~QMacSpinningProgressIndicatorPrivate()
    {
        [nsProgressIndicator release];
    }

    NSProgressIndicator *nsProgressIndicator;
};

QMacSpinningProgressIndicator::QMacSpinningProgressIndicator(QWidget *parent)
    : QWidget(parent), startTime(0)
{
    @autoreleasepool {
        NSProgressIndicator *progress = [[NSProgressIndicator alloc] init];
        [progress setStyle:NSProgressIndicatorSpinningStyle];

        pImpl.reset(new QMacSpinningProgressIndicatorPrivate(this, progress));

        parent->setAttribute(Qt::WA_NativeWindow);
        QHBoxLayout* layout = new QHBoxLayout(parent);
        layout->setMargin(0);
        layout->addWidget(new QMacCocoaViewContainer(progress, parent));
    }
}

QMacSpinningProgressIndicator::~QMacSpinningProgressIndicator()
{
}

void QMacSpinningProgressIndicator::animate(bool animate)
{
    assert(pImpl);

    if (!pImpl)
    {
        return;
    }

    if (animate)
    {
        [pImpl->nsProgressIndicator startAnimation:nil];
        startTime = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        [pImpl->nsProgressIndicator stopAnimation:nil];
        startTime = 0;
    }
}

void QMacSpinningProgressIndicator::start()
{
    animate(true);
}

void QMacSpinningProgressIndicator::stop()
{
    animate(false);
}

qint64 QMacSpinningProgressIndicator::getStartTime() const
{
    return startTime;
}
