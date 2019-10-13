#include "MenuItemAction.h"
#include <QKeyEvent>

MenuItemAction::MenuItemAction(const QString title, const QIcon icon, bool manageHoverStates, QSize iconSize)
    : QWidgetAction(NULL)
{
    this->title = new QLabel(title);
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

MenuItemAction::MenuItemAction(const QString title, const QString value, const QIcon icon, bool manageHoverStates, QSize iconSize)
    : QWidgetAction(NULL)
{
    this->title = new QLabel(title);
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

MenuItemAction::MenuItemAction(const QString title, const QIcon icon, const QIcon hoverIcon, bool manageHoverStates, QSize iconSize)
    : QWidgetAction(NULL)
{
    this->title = new QLabel(title);
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

void MenuItemAction::setLabelText(QString title)
{
    this->title->setText(title);
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
        container->setStyleSheet(QString::fromUtf8("#wContainer { border: 2px solid #aaaaaa; border-radius: 2px; margin: 0px 8px 0px 8px; padding: 0px; background-color: #aaaaaa; }"));
        title->setStyleSheet(QString::fromLatin1("font-family: Source Sans Pro; font-size: 14px; color: #ffffff;"));
        iconButton->setStyleSheet(QString::fromLatin1("border: none;"));
        hoverIcon ? iconButton->setIcon(*hoverIcon) : iconButton->setIcon(*icon);
    }
    else
    {
        container->setStyleSheet(QString::fromUtf8("#wContainer { border: none; margin: 0px 0px 0px 0px; padding: 0px; background-color: #ffffff; }"));
        title->setStyleSheet(QString::fromLatin1("font-family: Source Sans Pro; font-size: 14px; color: #777777;"));
        iconButton->setStyleSheet(QString::fromLatin1("border: none;"));
        iconButton->setIcon(*icon);
    }
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

void MenuItemAction::setupActionWidget(QSize iconSize)
{
    container->setMinimumHeight(32);
    container->setMaximumHeight(32);
    container->setStyleSheet(QString::fromLatin1("#wContainer { margin-left: 20px; padding: 0px; }"));

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
    iconButton->setFlat(true);

    title->setStyleSheet(QString::fromLatin1("font-family: Source Sans Pro; font-size: 14px; color: #777777;"));

    layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins(16, 0, 8, 0));
    layout->setSpacing(12);
    layout->addWidget(iconButton);
    layout->addWidget(title);
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    if (value)
    {
        value->setStyleSheet(QString::fromLatin1("font-family: Source Sans Pro; font-size: 14px; color: #777777; padding-right: 6px;"));
        layout->addWidget(value);
    }
    container->setLayout(layout);
}

bool MenuItemAction::eventFilter(QObject *obj, QEvent *event)
{
    if (!value)
    {
        if (event->type() == QEvent::Enter)
        {
            container->setStyleSheet(QString::fromUtf8("#wContainer { border: 2px solid #aaaaaa; border-radius: 2px; margin: 0px 8px 0px 8px; padding: 0px; background-color: #aaaaaa; }"));
            title->setStyleSheet(QString::fromLatin1("font-family: Source Sans Pro; font-size: 14px; color: #ffffff;"));
            iconButton->setStyleSheet(QString::fromLatin1("border: none;"));
            hoverIcon ? iconButton->setIcon(*hoverIcon) : iconButton->setIcon(*icon);
        }

        if (event->type() == QEvent::Leave)
        {
            container->setStyleSheet(QString::fromUtf8("#wContainer { border: none; margin: 0px 0px 0px 0px; padding: 0px; background-color: #ffffff; }"));
            title->setStyleSheet(QString::fromLatin1("font-family: Source Sans Pro; font-size: 14px; color: #777777;"));
            iconButton->setStyleSheet(QString::fromLatin1("border: none;"));
            iconButton->setIcon(*icon);
        }
    }

    return QWidgetAction::eventFilter(obj,event);
}
