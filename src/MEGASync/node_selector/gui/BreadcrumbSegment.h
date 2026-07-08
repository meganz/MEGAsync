#ifndef BREADCRUMBSEGMENT_H
#define BREADCRUMBSEGMENT_H

#include "Utilities.h" // ClickableLabel

#include <QSize>

class QPaintEvent;

// One breadcrumb path segment, rendered as a clickable label. Used by both navigation and
// destination breadcrumbs; the owner decides whether to wire its clicked() signal. A very long
// folder name is capped to MAX_WIDTH and painted with a middle ellipsis (full name as tooltip).
class BreadcrumbSegment: public ClickableLabel
{
    Q_OBJECT

public:
    explicit BreadcrumbSegment(QWidget* parent = nullptr);

    // Marks this segment as the current one (bold + text-primary color).
    void setHighlighted(bool highlighted);
    // Only toggles the cursor; wiring clicked() is the owner's responsibility.
    void setInteractive(bool interactive);
    // The first segment sits flush to the breadcrumb's left edge (drops its left margin).
    void setFirst(bool first);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    static constexpr int MAX_WIDTH = 280;
};

#endif // BREADCRUMBSEGMENT_H
