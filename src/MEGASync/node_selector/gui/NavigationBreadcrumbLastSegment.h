#ifndef NAVIGATIONBREADCRUMBLASTSEGMENT_H
#define NAVIGATIONBREADCRUMBLASTSEGMENT_H

#include <QFrame>

namespace Ui
{
class NavigationBreadcrumbLastSegment;
}

// Last navigation segment: a BreadcrumbSegment plus a chevron-down button, grouped under a
// surface-2 pill that toggles on while its context menu is open.
class NavigationBreadcrumbLastSegment: public QFrame
{
    Q_OBJECT

public:
    explicit NavigationBreadcrumbLastSegment(QWidget* parent = nullptr);
    ~NavigationBreadcrumbLastSegment() override;

    void setText(const QString& text);
    void setHighlighted(bool highlighted);

signals:
    void clicked();
    void menuRequested(const QPoint& globalPos);

private:
    void setMenuActive(bool active);

    Ui::NavigationBreadcrumbLastSegment* ui;
};

#endif // NAVIGATIONBREADCRUMBLASTSEGMENT_H
