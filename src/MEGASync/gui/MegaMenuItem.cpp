#include "MegaMenuItem.h"

#include "ThemeManager.h"
#include "TokenParserWidgetManager.h"
#include "Utilities.h"

MegaMenuItem::MegaMenuItem(const QString& text,
                           const QString& icon,
                           int treeDepth,
                           bool hasSubmenu,
                           QWidget* parent):
    QWidget(parent),
    m_text(text),
    m_iconName(icon),
    m_treeDepth(treeDepth),
    m_hasSubmenu(hasSubmenu),
    m_iconSpacing(8),
    m_textSpacing(0),
    m_beforeIconSpacing(0),
    m_enabled(true),
    m_hovered(false),
    m_pressed(false)
{
    // Create layout
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    // Create icon label
    m_iconLabel = new QLabel();
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(DEFAULT_ICON_SIZE,
                              DEFAULT_ICON_SIZE); // Default icon size

    // Create text label
    m_textLabel = new QLabel(text);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Create arrow label for submenu indicator
    m_arrowLabel = new QLabel();
    m_arrowLabel->setAlignment(Qt::AlignCenter);
    m_arrowLabel->setFixedSize(16, 16);
    createSubmenuArrow();

    updateLayout();
    updateStyleSheet();

    // Set default size
    setFixedHeight(32);
}

void MegaMenuItem::updateLayout()
{
    // Clear layout
    while (QLayoutItem* item = m_layout->takeAt(0))
    {
        delete item;
    }

    // Calculate margins based on tree depth
    int leftMargin = DEFAULT_BASE_MARGIN + (m_treeDepth * DEFAULT_TREE_INDENT);

    // Add before-icon spacing
    if (m_beforeIconSpacing > 0)
    {
        m_layout->addSpacing(m_beforeIconSpacing);
    }

    // Add icon if present
    if (!m_iconName.isEmpty())
    {
        QPixmap pixmap = QIcon(m_iconName).pixmap(DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE);
        m_iconLabel->setPixmap(pixmap);
        m_layout->addWidget(m_iconLabel);

        // Add spacing between icon and text
        if (m_iconSpacing > 0)
        {
            m_layout->addSpacing(m_iconSpacing);
        }
    }

    // Add text
    m_layout->addWidget(m_textLabel);

    // Add stretch to push submenu arrow to the right
    m_layout->addStretch();

    // Add submenu arrow if needed
    if (m_hasSubmenu)
    {
        m_layout->addWidget(m_arrowLabel);
        m_layout->addSpacing(0); // Small space from right edge
    }

    // Set margins (includes tree depth)
    m_layout->setContentsMargins(leftMargin, 4, 12, 4);
}

void MegaMenuItem::createSubmenuArrow()
{
    m_arrowLabel->setPixmap(
        Utilities::getCachedPixmap(QLatin1String("://chevron-right-icon-secondary.svg"))
            .pixmap(DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE));
    m_arrowLabel->setVisible(m_hasSubmenu);
}

void MegaMenuItem::setHasSubmenu(bool hasSubmenu)
{
    m_hasSubmenu = hasSubmenu;
    createSubmenuArrow();
    updateLayout();
}

void MegaMenuItem::updateStyleSheet()
{
    QString styleSheet;
    QString stateColor(QLatin1String("transparent"));
    if (m_enabled)
    {
        if (m_pressed)
        {
            stateColor = TokenParserWidgetManager::instance()
                             ->getColor(QLatin1String("surface-2"))
                             .name(QColor::NameFormat::HexArgb);
        }
        else if (m_hovered)
        {
            stateColor = TokenParserWidgetManager::instance()
                             ->getColor(QLatin1String("surface-1"))
                             .name(QColor::NameFormat::HexArgb);
        }
    }
    styleSheet = QLatin1String("MegaMenuItem {border-radius: 6px; background-color: %1; } QLabel "
                               "{background-color: %1;}")
                     .arg(stateColor);
    setStyleSheet(styleSheet);
    TokenParserWidgetManager::instance()->polish(this);
}

void MegaMenuItem::setTreeDepth(int depth)
{
    m_treeDepth = depth;
    updateLayout();
}

void MegaMenuItem::setText(const QString& text)
{
    m_text = text;
    m_textLabel->setText(text);
}

void MegaMenuItem::setIcon(const QString& icon)
{
    m_iconName = icon;
    updateLayout();
}

void MegaMenuItem::setEnabled(bool enabled)
{
    m_enabled = enabled;
    QWidget::setEnabled(enabled);
    updateStyleSheet();
}

void MegaMenuItem::mousePressEvent(QMouseEvent* event)
{
    if (m_enabled && event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        updateStyleSheet();
        emit pressed();
    }
    QWidget::mousePressEvent(event);
}

void MegaMenuItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_enabled && m_pressed && event->button() == Qt::LeftButton)
    {
        m_pressed = false;
        updateStyleSheet();
        emit clicked();
    }
    QWidget::mouseReleaseEvent(event);
}

void MegaMenuItem::enterEvent(QEvent* event)
{
    if (m_enabled)
    {
        m_hovered = true;
        updateStyleSheet();
        emit hovered();
    }
    QWidget::enterEvent(event);
}

void MegaMenuItem::leaveEvent(QEvent* event)
{
    if (m_enabled)
    {
        m_hovered = false;
        m_pressed = false;
        updateStyleSheet();
    }
    QWidget::leaveEvent(event);
}

void MegaMenuItem::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

bool MegaMenuItem::event(QEvent* event)
{
    if (event->type() == ThemeManager::ThemeChanged)
    {
        updateLayout();
    }
    return QWidget::event(event);
}

void MegaMenuItem::resetPressedState()
{
    m_pressed = false;
    m_hovered = false;
    updateStyleSheet();
}
