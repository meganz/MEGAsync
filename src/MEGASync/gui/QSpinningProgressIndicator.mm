#include "QSpinningProgressIndicator.h"

#import "Foundation/NSAutoreleasePool.h"
#import "AppKit/NSProgressIndicator.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QMacCocoaViewContainer>


class QSpinningProgressIndicatorPrivate : public QObject
{
public:
    QSpinningProgressIndicatorPrivate(QSpinningProgressIndicator *qProgressIndicatorSpinning,
                                      NSProgressIndicator *nsProgressIndicator)
        : QObject(qProgressIndicatorSpinning), nsProgressIndicator(nsProgressIndicator) {}


    ~QSpinningProgressIndicatorPrivate()
    {
        [nsProgressIndicator release];
    }

    NSProgressIndicator *nsProgressIndicator;
};

QSpinningProgressIndicator::QSpinningProgressIndicator(QWidget *parent)
    : QWidget(parent), startTime(0)
{
    @autoreleasepool {
        NSProgressIndicator *progress = [[NSProgressIndicator alloc] init];
        [progress setStyle:NSProgressIndicatorSpinningStyle];

        pImpl.reset(new QSpinningProgressIndicatorPrivate(this, progress));

        parent->setAttribute(Qt::WA_NativeWindow);
        QHBoxLayout* layout = new QHBoxLayout(parent);
        layout->setMargin(0);
        layout->addWidget(new QMacCocoaViewContainer(progress, parent));
    }
}

QSpinningProgressIndicator::~QSpinningProgressIndicator()
{
}

void QSpinningProgressIndicator::animate(bool animate)
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

qint64 QSpinningProgressIndicator::getStartTime() const
{
    return startTime;
}
