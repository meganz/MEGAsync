#include "NavigationBreadcrumbLastSegment.h"

#include "BreadcrumbSegment.h"
#include "ui_NavigationBreadcrumbLastSegment.h"
#include "Utilities.h"

#include <QStyle>
#include <QToolButton>

NavigationBreadcrumbLastSegment::NavigationBreadcrumbLastSegment(QWidget* parent):
    QFrame(parent),
    ui(new Ui::NavigationBreadcrumbLastSegment)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    ui->chevron->setIcon(QIcon(Utilities::getPixmapName(QLatin1String("chevron_down"),
                                                        Utilities::AttributeType::SMALL |
                                                            Utilities::AttributeType::THIN |
                                                            Utilities::AttributeType::OUTLINE)));

    // The last segment is always interactive (carries the chevron + context menu).
    // The frame's layout already provides the 8,3,8,3 padding (and its background fills the
    // whole pill from edge to edge), so the inner segment must not add its own margins.
    ui->segment->setContentsMargins(0, 0, 0, 0);
    ui->segment->setInteractive(true);

    connect(ui->segment, &ClickableLabel::clicked, this, &NavigationBreadcrumbLastSegment::clicked);

    connect(ui->chevron,
            &QToolButton::clicked,
            this,
            [this]()
            {
                // The pill stays on only while the (synchronous, blocking) menu is open.
                setMenuActive(true);
                emit menuRequested(mapToGlobal(QPoint(0, height())));
                setMenuActive(false);
            });
}

NavigationBreadcrumbLastSegment::~NavigationBreadcrumbLastSegment()
{
    delete ui;
}

void NavigationBreadcrumbLastSegment::setText(const QString& text)
{
    ui->segment->setText(text);
}

void NavigationBreadcrumbLastSegment::setHighlighted(bool highlighted)
{
    ui->segment->setHighlighted(highlighted);
}

void NavigationBreadcrumbLastSegment::setMenuActive(bool active)
{
    setProperty("menuActive", active);
    style()->unpolish(this);
    style()->polish(this);
    update();
}
