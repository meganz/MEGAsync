#include "NavigationBreadcrumb.h"

#include "Breadcrumb.h"
#include "BreadcrumbSegment.h"
#include "NavigationBreadcrumbLastSegment.h"
#include "ui_NavigationBreadcrumb.h"

NavigationBreadcrumb::NavigationBreadcrumb(QWidget* parent):
    QFrame(parent),
    ui(new Ui::NavigationBreadcrumb)
{
    ui->setupUi(this);

    // Navigation segments keep their full 8px side padding: the navigation row's left margin
    // is reduced to 12px so the first segment's text still lands on the 20px content line,
    // while its highlight pill extends 8px to the left of it.
    ui->breadcrumb->setSegmentFactory(
        [this](const QString& text, int index, bool /*isFirst*/, bool isLast) -> QWidget*
        {
            // The last segment carries the chevron + context menu.
            if (isLast)
            {
                // A read-only root has no context-menu actions, and an empty root already
                // offers them on the empty page. In both cases render the last segment as a
                // plain highlighted segment without the chevron + menu affordance.
                if (!mLastSegmentInteractive)
                {
                    auto* segment = new BreadcrumbSegment;
                    segment->setText(text);
                    segment->setHighlighted(true);
                    segment->setInteractive(false);
                    return segment;
                }

                auto* lastSegment = new NavigationBreadcrumbLastSegment;
                lastSegment->setText(text);
                lastSegment->setHighlighted(true);

                connect(lastSegment,
                        &NavigationBreadcrumbLastSegment::clicked,
                        this,
                        [this, index]()
                        {
                            emit segmentActivated(index);
                        });
                connect(lastSegment,
                        &NavigationBreadcrumbLastSegment::menuRequested,
                        this,
                        &NavigationBreadcrumb::lastSegmentMenuRequested);

                return lastSegment;
            }

            auto* segment = new BreadcrumbSegment;
            segment->setText(text);
            segment->setHighlighted(false);
            segment->setInteractive(true);
            connect(segment,
                    &ClickableLabel::clicked,
                    this,
                    [this, index]()
                    {
                        emit segmentActivated(index);
                    });
            return segment;
        });

    connect(ui->breadcrumb,
            &Breadcrumb::overflowSegmentActivated,
            this,
            &NavigationBreadcrumb::segmentActivated);
    connect(ui->breadcrumb, &Breadcrumb::refreshNeeded, this, &NavigationBreadcrumb::refreshNeeded);
}

NavigationBreadcrumb::~NavigationBreadcrumb()
{
    delete ui;
}

void NavigationBreadcrumb::setSegments(const QList<NodeSelectorBreadcrumbSegment>& segments,
                                       bool lastSegmentInteractive)
{
    if (mLastSegmentInteractive != lastSegmentInteractive)
    {
        ui->breadcrumb->clear();
    }

    mLastSegmentInteractive = lastSegmentInteractive;

    ui->breadcrumb->setSegments(segments, true);
}

void NavigationBreadcrumb::onNodesRenamed(const QList<mega::MegaHandle>& handles)
{
    ui->breadcrumb->onNodesRenamed(handles);
}
