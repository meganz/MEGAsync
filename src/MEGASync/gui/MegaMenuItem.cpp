#include "MegaMenuItem.h"

#include "ThemeManager.h"
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
    m_beforeIconSpacing(0)
{
    // Make transparent for mouse events - let QMenu handle all interaction
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // Transparent background
    setStyleSheet(QLatin1String("MegaMenuItem { background-color: transparent; }"));

    // Create layout
    m_layout = new QHBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // Create icon label
    m_iconLabel = new IconLabel(this);
    m_iconLabel->setProperty("disabled_off", QLatin1String("button-disabled"));
    m_iconLabel->setFixedSize(DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE);

    // Create text label
    m_textLabel = new QLabel(text, this);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textLabel->setStyleSheet(QLatin1String("background-color: transparent"));
    // Create arrow label for submenu indicator
    m_arrowLabel = new QLabel(this);
    m_arrowLabel->setAlignment(Qt::AlignCenter);
    m_arrowLabel->setFixedSize(16, 16);
    m_arrowLabel->setStyleSheet(QLatin1String("background-color: transparent"));
    createSubmenuArrow();

    updateLayout();

    // Set default size
    setFixedHeight(32);

    parent->installEventFilter(this);
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
        auto token = parent()->property("icon-token").toString();

        if (token.isEmpty())
        {
            m_iconLabel->setIcon(QIcon(m_iconName).pixmap(DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE));
        }
        else
        {
            m_iconLabel->setIcon(Utilities::getIcon(m_iconName, Utilities::AttributeType::NONE));
            m_iconLabel->setProperty("normal_off", token);
        }

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
        m_layout->addSpacing(0);
    }

    // Set margins (includes tree depth)
    m_layout->setContentsMargins(leftMargin, 4, 12, 4);
}

void MegaMenuItem::createSubmenuArrow()
{
    if (m_arrowLabel != nullptr)
    {
        m_arrowLabel->setPixmap(
            Utilities::getCachedPixmap(QLatin1String("://chevron-right-icon-secondary.svg"))
                .pixmap(DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE));
        m_arrowLabel->setVisible(m_hasSubmenu);
    }
}

void MegaMenuItem::setHasSubmenu(bool hasSubmenu)
{
    m_hasSubmenu = hasSubmenu;
    createSubmenuArrow();
    updateLayout();
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

bool MegaMenuItem::event(QEvent* event)
{
    if (event->type() == ThemeManager::ThemeChanged)
    {
        updateLayout();
        update();
    }

    return QWidget::event(event);
}

bool MegaMenuItem::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange)
    {
        auto* dynamicPropertyEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
        if (dynamicPropertyEvent != nullptr)
        {
            if (QString::fromLatin1(dynamicPropertyEvent->propertyName()) ==
                QLatin1String("icon-token"))
            {
                updateLayout();
                update();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
