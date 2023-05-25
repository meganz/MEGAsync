#include "MenuItemAction.h"
#include <QKeyEvent>
#include <QStyle>
#include <QImageReader>

const QString MenuItemAction::Colors::Normal = QLatin1String("#777777");
const QString MenuItemAction::Colors::Highlight = QLatin1String("#000000");
const QString MenuItemAction::Colors::Accent = QLatin1String("#F46265");
static constexpr int ENTRY_MAX_WIDTH_PX = 240;

MenuItemAction::MenuItemAction(const QString& title, const QString& icon,
               QObject *parent)
    : QWidgetAction (parent),
      mAccent(false),
      mContainer (new QWidget()),
      mTitle (new QLabel(mContainer)),
      mValue(nullptr),
      mIconButton (new QPushButton(mContainer)),
      mActionLayout(nullptr)
{
    setLabelText(title);
    mContainer->setObjectName(QLatin1String("wContainer"));
    mContainer->installEventFilter(this);

    QSize iconSize(24,24);

    if(!icon.isEmpty())
    {
        QImageReader reader(icon);
        if(reader.canRead())
        {
            iconSize = reader.size();
            mIcon = QIcon(icon);
        }
    }

    setupActionWidget(iconSize);
    setDefaultWidget(mContainer);
}

void MenuItemAction::setIcon(const QIcon& icon)
{
    mIcon = icon;
    mIconButton->setIcon(mIcon);
}

void MenuItemAction::setHoverIcon(const QIcon& icon)
{
    mHoverIcon = icon;
}

void MenuItemAction::setHighlight(bool highlight)
{   
    if (highlight)
    {
        mTitle->setStyleSheet(QString::fromLatin1("color: %1;").arg(Colors::Highlight));
    }
    else
    {
        mTitle->setStyleSheet(QString::fromLatin1("color: %1;").arg(getColor()));
    }
}

MenuItemAction::~MenuItemAction()
{
    QLayout* layout (mContainer->layout());
    QLayoutItem* child;
    while ((child = layout->takeAt(0)) != 0)
    {
        delete child->widget();
        delete child;
    }
    mContainer->deleteLater();
}

void MenuItemAction::setManagesHoverStates(bool managesHoverStates)
{
    if (managesHoverStates)
    {
        mContainer->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

void MenuItemAction::setTreeDepth(int treeDepth)
{
    mActionLayout->setContentsMargins(QMargins(16 + treeDepth * 20, 0, 16, 0));
}

void MenuItemAction::setLabelText(const QString& title)
{
    setObjectName(title);
    // Force polish to update font Info with .ui StyleSheet
    this->mTitle->ensurePolished();
    auto f (this->mTitle->fontMetrics());
    QString elidedTitle (f.elidedText(title, Qt::ElideMiddle, ENTRY_MAX_WIDTH_PX));
    this->mTitle->setText(elidedTitle);
    if (title != elidedTitle)
    {
        this->setToolTip(title);
    }
}

void MenuItemAction::setupActionWidget(const QSize& iconSize)
{
    mContainer->setMinimumHeight(32);
    mContainer->setMaximumHeight(32);
    mContainer->setStyleSheet(QLatin1String("#wContainer { margin-left: 20px; padding: 0px; }"
                                            "QLabel {font-family: Lato; font-size: 14px;}"
                                            "QPushButton { border: none; }"));

    mIconButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mIconButton->setText(QString());
    mIconButton->setFixedSize(iconSize);
    mIconButton->setIconSize(iconSize);
    mIconButton->setIcon(mIcon);

    mTitle->setStyleSheet(QString::fromLatin1("color: %1;").arg(getColor()));

    mActionLayout = new QHBoxLayout();
    mActionLayout->setContentsMargins(QMargins(16, 0, 16, 0));
    mActionLayout->setSpacing(12);
    mActionLayout->addWidget(mIconButton, 0, Qt::AlignVCenter);
    mActionLayout->addWidget(mTitle, 0, Qt::AlignVCenter);

    if (mValue)
    {
        mActionLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
        mValue->setStyleSheet(QString::fromLatin1("color: %1;").arg(getColor()));
        mActionLayout->addWidget(mValue);
    }
    mContainer->setLayout(mActionLayout);
}

bool MenuItemAction::eventFilter(QObject *obj, QEvent *event)
{
    if (!mValue && isEnabled())
    {
        if (event->type() == QEvent::Enter)
        {
            mTitle->setStyleSheet(QString::fromLatin1("color: %1;").arg(Colors::Highlight));
        }

        if (event->type() == QEvent::Leave)
        {
            mTitle->setStyleSheet(QString::fromLatin1("color: %1;").arg(getColor()));
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
    {
        mTitle->setStyleSheet(QString::fromLatin1("color: %1;").arg(getColor()));
    }
}

const QString& MenuItemAction::getColor() const
{
    if(mAccent)
        return Colors::Accent;
    else
        return Colors::Normal;
}

