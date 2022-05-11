#include "BalloonToolTip.h"
#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

static constexpr int TAIL_SIZE_PX (8);

BalloonToolTip::BalloonToolTip(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    animation.setTargetObject(this);
    animation.setPropertyName("popupOpacity");
    animation.setDuration(150);

    label.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    label.setStyleSheet(QLatin1String("QLabel {color : #FFFFFF; font-size: 12px;}"));
    label.setWordWrap(true);

    layout.setHorizontalSpacing(0);
    layout.setVerticalSpacing(0);

    layout.setContentsMargins(12, 10, 12, 10 + TAIL_SIZE_PX);

    layout.addWidget(&label, 0, 0);
    setLayout(&layout);

    installEventFilter(this);
    setMouseTracking(true);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
}

BalloonToolTip::~BalloonToolTip()
{
}

void BalloonToolTip::setPopupText(const QString &text)
{
    label.setText(text);
    adjustSize();
}

void BalloonToolTip::show()
{
    setWindowOpacity(0.0);

    animation.setStartValue(0.0);
    animation.setEndValue(1.0);

    QWidget::show();

    animation.start();
}

void BalloonToolTip::setPopupOpacity(float opacity)
{
    popupOpacity = opacity;
    setWindowOpacity(opacity);
}

float BalloonToolTip::getPopupOpacity() const
{
    return popupOpacity;
}

// Move widget so that the tail's tip matches p. p is in global coords.
void BalloonToolTip::attachAt(const QPoint& p)
{
    move(p - QPoint(rect().center().x(), rect().bottom()));
}

void BalloonToolTip::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Popup dimensions
    QRect roundedRectDimensions (rect());
    roundedRectDimensions.setBottom(roundedRectDimensions.bottom() - TAIL_SIZE_PX);

    const QColor balloonColor (8, 33, 44);
    painter.setBrush(QBrush(balloonColor));

    QPen pen;
    pen.setColor(balloonColor);
    pen.setWidth(1);
    painter.setPen(pen);

    // Draw the popup balloon
    painter.drawRoundedRect(roundedRectDimensions, 8, 8);

    painter.setPen(Qt::NoPen);
    painter.setBrush(balloonColor);

    // Draw the popup tail
    const auto tailTipBaseX (roundedRectDimensions.center().x());
    const auto tailTipBaseY (roundedRectDimensions.height());
    const QPointF points[3] = {
        QPoint(tailTipBaseX + TAIL_SIZE_PX, tailTipBaseY),
        QPoint(tailTipBaseX - TAIL_SIZE_PX, tailTipBaseY),
        QPoint(tailTipBaseX, tailTipBaseY + TAIL_SIZE_PX)
    };

    painter.drawPolygon(points, sizeof(points) / sizeof(QPointF));
}

bool BalloonToolTip::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o)

    switch (e->type()) {
    #ifdef Q_OS_MACOS
        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            const int key = static_cast<QKeyEvent *>(e)->key();
            // Anything except key modifiers or caps-lock, etc.
            if (key < Qt::Key_Shift || key > Qt::Key_ScrollLock)
                hide();
            break;
        }
    #endif
        case QEvent::Leave:
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Close: // For QTBUG-55523 (QQC) specifically: Hide tooltip when windows are closed
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Wheel:
            hide();
            break;
        case QEvent::MouseMove:
            if (!rect().isNull() && !rect().contains(static_cast<QMouseEvent*>(e)->pos()))
                hide();
        default:
            break;
        }
        return false;
    }
