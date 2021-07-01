#include "MenuItemAction.h"
#include <QKeyEvent>

MenuItemAction::MenuItemAction(const QString& title, const QString& value,
                               const QIcon icon, const QIcon hoverIcon, bool manageHoverStates,
                               int treeDepth, QSize iconSize)
    : QWidgetAction (nullptr),
      mContainer (new QWidget(nullptr)),
      mIcon (new QIcon(icon)),
      mHoverIcon (new QIcon(hoverIcon)),
      mTitle (new QLabel(title)),
      mValue (value.isNull() ? nullptr : new QLabel(value)),
      mTreeDepth (treeDepth)
{
    mContainer->setObjectName(QString::fromUtf8("wContainer"));
    mContainer->installEventFilter(this);

    if (manageHoverStates)
    {
        mContainer->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    setupActionWidget(iconSize);
    setDefaultWidget(mContainer);
}

MenuItemAction::MenuItemAction(const QString& title, const QIcon icon,
                               bool manageHoverStates, int treeDepth, QSize iconSize)
    : MenuItemAction (title, QString(), icon, QIcon(), manageHoverStates, treeDepth, iconSize)
{
}

MenuItemAction::MenuItemAction(const QString& title, const QString& value, const QIcon icon,
                               bool manageHoverStates, int treeDepth, QSize iconSize)
    : MenuItemAction (title, value, icon,  QIcon(), manageHoverStates, treeDepth, iconSize)
{
}

MenuItemAction::MenuItemAction(const QString& title, const QIcon icon, const QIcon hoverIcon,
                               bool manageHoverStates, int treeDepth, QSize iconSize)
    : MenuItemAction (title, QString(), icon, hoverIcon, manageHoverStates, treeDepth, iconSize)
{
}

void MenuItemAction::setLabelText(const QString& title)
{
    mTitle->setText(title);
}

void MenuItemAction::setIcon(const QIcon icon)
{
    delete mIcon;
    mIcon = new QIcon(icon);
    mIconButton->setIcon(*mIcon);
}

void MenuItemAction::setHoverIcon(const QIcon icon)
{
    delete mHoverIcon;
    mHoverIcon = new QIcon(icon);
}

void MenuItemAction::setHighlight(bool highlight)
{   
    mTitle->setStyleSheet(QString::fromUtf8("font-family: Lato; font-size: 14px; color: %1;")
                          .arg(highlight ? QString::fromUtf8("#000000")
                                         : QString::fromUtf8("#777777")));
}

MenuItemAction::~MenuItemAction()
{
    delete mTitle;
    delete mValue;
    delete mIconButton;
    delete mIcon;
    delete mHoverIcon;
    delete mLayout;
    delete mContainer;
}

void MenuItemAction::setupActionWidget(QSize iconSize)
{
    mContainer->setMinimumHeight(32);
    mContainer->setMaximumHeight(32);
    mContainer->setStyleSheet(QString::fromUtf8("#wContainer { margin-left: 20px; padding: 0px; }"));

    mIconButton = new QPushButton();
    mIconButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mIconButton->setText(QString());
    mIconButton->setMinimumSize(iconSize);
    mIconButton->setMaximumSize(iconSize);
    const QList<QSize> sizes = mIcon->availableSizes();
    if (!sizes.empty())
    {
        mIconButton->setIconSize(sizes.at(0));
    }
    mIconButton->setIcon(*mIcon);
    mIconButton->setFlat(true);

    mTitle->setStyleSheet(QString::fromUtf8("font-family: Lato; font-size: 14px; color: #777777;"));

    mLayout = new QHBoxLayout();
    mLayout->setContentsMargins(QMargins(16 + mTreeDepth * 33, 0, 8, 0));
    mLayout->setSpacing(12);
    mLayout->addWidget(mIconButton);
    mLayout->addWidget(mTitle);
    mLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    if (mValue)
    {
        mValue->setStyleSheet(QString::fromUtf8("font-family: Lato; font-size: 14px;"
                                                "color: #777777; padding-right: 6px;"));
        mLayout->addWidget(mValue);
    }
    mContainer->setLayout(mLayout);
}

bool MenuItemAction::eventFilter(QObject *obj, QEvent *event)
{
    if (!mValue)
    {
        if (event->type() == QEvent::Enter)
        {
            mTitle->setStyleSheet(QString::fromUtf8("font-family: Lato; font-size: 14px;"
                                                    "color: #000000;"));
        }

        if (event->type() == QEvent::Leave)
        {
            mTitle->setStyleSheet(QString::fromUtf8("font-family: Lato; font-size: 14px;"
                                                    "color: #777777;"));
        }
    }

    return QWidgetAction::eventFilter(obj,event);
}
