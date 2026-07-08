#include "NodeSelectorDestinationOverflowPopup.h"

#include "TokenParserWidgetManager.h"
#include "ui_NodeSelectorDestinationOverflowPopup.h"
#include "Utilities.h"

#include <QLabel>
#include <QLayoutItem>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QVector>

#include <algorithm>

NodeSelectorDestinationOverflowPopup::NodeSelectorDestinationOverflowPopup(QWidget* parent):
    QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
    ui(new Ui::NodeSelectorDestinationOverflowPopup)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

NodeSelectorDestinationOverflowPopup::~NodeSelectorDestinationOverflowPopup()
{
    delete ui;
}

void NodeSelectorDestinationOverflowPopup::setSegments(const QStringList& segments, int indexOffset)
{
    clearLabels();

    auto* layout = qobject_cast<QVBoxLayout*>(ui->scrollContent->layout());
    if (!layout)
    {
        return;
    }

    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    static constexpr int MAX_VISIBLE_ROWS = 5;
    static constexpr int ROW_HEIGHT = 26;
    static constexpr int POPUP_BORDER_WIDTH = 1;

    QVector<QWidget*> rowWidgets;
    rowWidgets.reserve(segments.size());

    const bool clickable = indexOffset >= 0;

    for (int i = 0; i < segments.size(); ++i)
    {
        // Always a label (same format as the breadcrumb row); clickable rows use a
        // ClickableLabel so navigation segments emit a click without resorting to buttons.
        QLabel* label = nullptr;
        if (clickable)
        {
            auto* clickableLabel = new ClickableLabel(ui->scrollContent);
            clickableLabel->setCursor(Qt::PointingHandCursor);
            clickableLabel->setProperty("clickable", true);
            // QLabel ignores :hover; ClickableLabel drives the row highlight via [hovered="true"].
            clickableLabel->setHoverHighlightEnabled(true);
            connect(clickableLabel,
                    &ClickableLabel::clicked,
                    this,
                    [this, segmentIndex = indexOffset + i]()
                    {
                        emit segmentActivated(segmentIndex);
                    });
            label = clickableLabel;
        }
        else
        {
            label = new QLabel(ui->scrollContent);
        }

        label->setText(segments.at(i));
        label->setFixedHeight(ROW_HEIGHT);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        label->setContentsMargins(0, 0, 0, 0);
        label->setMargin(0);
        label->setIndent(0);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        // Long names are elided to MAX_ROW_WIDTH below; shorter ones keep their natural width.
        label->setProperty("font-size", QLatin1String("body-2"));
        label->setProperty("regular", true);

        layout->addWidget(label);
        rowWidgets.push_back(label);
    }

    TokenParserWidgetManager::instance()->applyCurrentTheme(this);

    layout->activate();

    static constexpr int MAX_ROW_WIDTH = 280;

    int maxRowWidth = 0;
    for (int i = 0; i < rowWidgets.size(); ++i)
    {
        auto* label = qobject_cast<QLabel*>(rowWidgets.at(i));
        if (!label)
        {
            continue;
        }

        label->ensurePolished();

        // Cap a very long ancestor name with a middle ellipsis (full name kept as tooltip) so a
        // single long segment can't blow up the popup width.
        const QString fullText = segments.at(i);
        const QString elided =
            label->fontMetrics().elidedText(fullText, Qt::ElideMiddle, MAX_ROW_WIDTH);
        if (elided != fullText)
        {
            label->setText(elided);
            label->setToolTip(fullText);
        }

        maxRowWidth = (std::max)(maxRowWidth, label->sizeHint().width());
    }

    const int rowCount = segments.size();
    const int visibleRows = (std::min)(MAX_VISIBLE_ROWS, rowCount);
    const bool needsScrollbar = rowCount > MAX_VISIBLE_ROWS;
    const auto margins = layout->contentsMargins();
    const int spacing = layout->spacing();
    const int verticalMargins = margins.top() + margins.bottom();

    // Inner content holds ALL rows (so it can scroll); the viewport is capped at MAX_VISIBLE_ROWS.
    const int fullContentHeight =
        verticalMargins + rowCount * ROW_HEIGHT + (std::max)(0, rowCount - 1) * spacing;
    const int viewportHeight =
        verticalMargins + visibleRows * ROW_HEIGHT + (std::max)(0, visibleRows - 1) * spacing;

    const int contentWidth = margins.left() + margins.right() + maxRowWidth;
    const int scrollbarWidth =
        needsScrollbar ? ui->scrollArea->verticalScrollBar()->sizeHint().width() : 0;
    const int popupWidth = contentWidth + scrollbarWidth + 2 * POPUP_BORDER_WIDTH;
    const int popupHeight = viewportHeight + 2 * POPUP_BORDER_WIDTH;

    for (auto* rowWidget: rowWidgets)
    {
        rowWidget->setFixedWidth(maxRowWidth);
    }

    ui->scrollArea->setVerticalScrollBarPolicy(needsScrollbar ? Qt::ScrollBarAsNeeded :
                                                                Qt::ScrollBarAlwaysOff);
    ui->scrollContent->setFixedSize(contentWidth, fullContentHeight);
    ui->scrollArea->setFixedSize(popupWidth, popupHeight);
    setFixedSize(popupWidth, popupHeight);
}

void NodeSelectorDestinationOverflowPopup::clearLabels()
{
    auto* layout = qobject_cast<QVBoxLayout*>(ui->scrollContent->layout());
    if (!layout)
    {
        return;
    }

    while (auto* item = layout->takeAt(0))
    {
        if (auto* w = item->widget())
        {
            w->deleteLater();
        }
        delete item;
    }
}
