#include "MenuItemAction.h"
#include <QKeyEvent>
#include <QStyle>

const QString MenuItemAction::Colors::Normal = QLatin1String("#777777");
const QString MenuItemAction::Colors::Highlight = QLatin1String("#000000");
const QString MenuItemAction::Colors::Accent = QLatin1String("#F46265");
static constexpr int ENTRY_MAX_WIDTH_PX = 240;

MenuItemAction::MenuItemAction(const QString title, const QIcon icon, bool manageHoverStates, QSize iconSize, bool accent)
    : QWidgetAction(NULL), mAccent(accent)
{
    this->title = new QLabel();
    setLabelText(title);
    this->icon = new QIcon(icon);
    this->hoverIcon = NULL;
    this->value = NULL;
    container = new QWidget(NULL);
    container->setObjectName(QString::fromUtf8("wContainer"));
    container->installEventFilter(this);

    if (manageHoverStates)
    {
        container->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    setupActionWidget(iconSize);
    setDefaultWidget(container);
}

MenuItemAction::MenuItemAction(const QString title, const QString value, const QIcon icon, bool manageHoverStates, QSize iconSize, bool accent)
    : QWidgetAction(NULL), mAccent(accent)
{
    this->title = new QLabel();
    setLabelText(title);
    this->value = new QLabel(value);
    this->icon = new QIcon(icon);
    this->hoverIcon = NULL;
    container = new QWidget(NULL);
    container->setObjectName(QString::fromUtf8("wContainer"));
    container->installEventFilter(this);

    if (manageHoverStates)
    {
        container->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    setupActionWidget(iconSize);
    setDefaultWidget(container);
}

MenuItemAction::MenuItemAction(const QString title, const QIcon icon, const QIcon hoverIcon, bool manageHoverStates, QSize iconSize, bool accent)
    : QWidgetAction(NULL), mAccent(accent)
{
    this->title = new QLabel();
    setLabelText(title);
    this->icon = new QIcon(icon);
    this->hoverIcon = new QIcon(hoverIcon);
    this->value = NULL;
    container = new QWidget (NULL);
    container->setObjectName(QString::fromUtf8("wContainer"));
    container->installEventFilter(this);

    if (manageHoverStates)
    {
        container->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    setupActionWidget(iconSize);
    setDefaultWidget(container);
}

MenuItemAction::~MenuItemAction()
{
    delete title;
    delete value;
    delete iconButton;
    delete icon;
    delete hoverIcon;
    delete layout;
    delete container;
}


void MenuItemAction::setLabelText(QString title)
{
    // Force polish to update font Info with .ui StyleSheet
    this->title->ensurePolished();
    auto f (this->title->fontMetrics());
    QString elidedTitle (f.elidedText(title, Qt::ElideMiddle, ENTRY_MAX_WIDTH_PX));
    this->title->setText(elidedTitle);
    if (title != elidedTitle)
    {
        this->setToolTip(title);
    }
}

void MenuItemAction::setIcon(const QIcon icon)
{
    delete this->icon;
    this->icon = new QIcon(icon);
    iconButton->setIcon(*(this->icon));
}

void MenuItemAction::setHoverIcon(const QIcon icon)
{
    delete this->hoverIcon;
    this->hoverIcon = new QIcon(icon);
}

void MenuItemAction::setHighlight(bool highlight)
{
    if (highlight)
    {
        title->setStyleSheet(QString::fromAscii("color: %1;").arg(Colors::Highlight));
    }
    else
    {
        title->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));
    }
}

void MenuItemAction::setupActionWidget(QSize iconSize)
{
    container->setMinimumHeight(32);
    container->setMaximumHeight(32);
    container->setStyleSheet(QString::fromAscii("#wContainer { margin-left: 20px; padding: 0px; } QLabel {font-family: Lato; font-size: 14px;}"
                                                "QPushButton { border: none; }"));

    iconButton = new QPushButton();
    iconButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    iconButton->setText(QString::fromUtf8(""));
    iconButton->setMinimumSize(iconSize);
    iconButton->setMaximumSize(iconSize);
    const QList<QSize> sizes = icon->availableSizes();
    if (!sizes.empty())
    {
        iconButton->setIconSize(sizes.at(0));
    }
    iconButton->setIcon(*icon);

    title->setParent(container);
    title->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));

    layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(16, 0, 8, 0));
    layout->setSpacing(12);
    layout->addWidget(iconButton);
    layout->addWidget(title);
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    if (value)
    {
        value->setStyleSheet(QString::fromAscii("padding-right: 6px; color: %1;").arg(getColor()));
        layout->addWidget(value);
    }
    container->setLayout(layout);
}

QString MenuItemAction::getColor()
{
    if(mAccent)
        return Colors::Accent;
    else
        return Colors::Normal;
}

bool MenuItemAction::eventFilter(QObject *obj, QEvent *event)
{
    if (!value)
    {
        if (event->type() == QEvent::Enter)
        {
            title->setStyleSheet(QString::fromAscii("color: %1;").arg(Colors::Highlight));
        }

        if (event->type() == QEvent::Leave)
        {
            title->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));
        }
    }

    return QWidgetAction::eventFilter(obj,event);
}

bool MenuItemAction::getAccent() const
{
    return mAccent;
}

void MenuItemAction::setAccent(bool enabled)
{
    mAccent = enabled;
    if(title)
        title->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));
}
