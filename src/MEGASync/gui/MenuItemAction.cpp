#include "MenuItemAction.h"
#include <QKeyEvent>

const QString MenuItemAction::Colors::Normal = QLatin1String("#777777");
const QString MenuItemAction::Colors::Highlight = QLatin1String("#000000");
const QString MenuItemAction::Colors::Accent = QLatin1String("#F46265");

MenuItemAction::MenuItemAction(const QString& title, const QString& value,
                               const QIcon icon, const QIcon hoverIcon, bool manageHoverStates,
                               int treeDepth, QSize iconSize)
    : QWidgetAction (nullptr),
      mAccent(false),
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
    if (highlight)
    {
        mTitle->setStyleSheet(QString::fromAscii("color: %1;").arg(Colors::Highlight));
    }
    else
    {
        mTitle->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));
    }
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
    mContainer->setStyleSheet(QString::fromLatin1("#wContainer { margin-left: 20px; padding: 0px; } QLabel {font-family: Lato; font-size: 14px;}"));

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

    mTitle->setParent(mContainer);
    mTitle->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));

    mLayout = new QHBoxLayout();
    mLayout->setContentsMargins(QMargins(16 + mTreeDepth * 33, 0, 8, 0));
    mLayout->setSpacing(12);
    mLayout->addWidget(mIconButton);
    mLayout->addWidget(mTitle);
    mLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    if (mValue)
    {
        mValue->setStyleSheet(QString::fromAscii("padding-right: 6px; color: %1;").arg(getColor()));
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
            mTitle->setStyleSheet(QString::fromAscii("color: %1;").arg(Colors::Highlight));
        }

        if (event->type() == QEvent::Leave)
        {
            mTitle->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));
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
    if(mTitle)
        mTitle->setStyleSheet(QString::fromAscii("color: %1;").arg(getColor()));
}

QString MenuItemAction::getColor()
{
    if(mAccent)
        return Colors::Accent;
    else
        return Colors::Normal;
}

