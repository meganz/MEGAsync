#include "BannerWidget.h"

#include "ui_BannerWidget.h"
#include "Utilities.h"

#include <QStyle>
namespace
{
constexpr const char* TYPE_PROPERTY_NAME{"type"};
const std::map<BannerWidget::Type, QLatin1String> TYPE_MAP{
    {BannerWidget::Type::BANNER_WARNING, QLatin1String{"warning"}},
    {BannerWidget::Type::BANNER_ERROR,   QLatin1String{"error"}  },
    {BannerWidget::Type::BANNER_INFO,    QLatin1String{"info"}   }
};
}

BannerWidget::BannerWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::BannerWidget),
    mType(Type::NONE)
{
    mUi->setupUi(this);
    mUi->wLinkContainer->hide();
    mUi->lTitle->hide();
    mUi->lText->hide();
    mUi->lText->setTextFormat(Qt::RichText);
    mUi->lText->setKeepParentCursor(false);

    connect(mUi->bLink,
            &QPushButton::clicked,
            this,
            &BannerWidget::linkActivated,
            Qt::UniqueConnection);
}

BannerWidget::~BannerWidget()
{
    delete mUi;
}

void BannerWidget::setType(Type type)
{
    if (mType == type)
    {
        return;
    }

    mType = type;

    auto it = TYPE_MAP.find(mType);
    if (it == TYPE_MAP.end())
    {
        return;
    }

    QLatin1String typeString(it->second);
    mUi->wContent->setProperty(TYPE_PROPERTY_NAME, typeString);

    // Force update the style of all child widgets.
    mUi->wContent->setStyleSheet(this->styleSheet());
}

void BannerWidget::setDescription(const QString& text)
{
    mUi->lText->setText(text);
    mUi->lText->show();
}

void BannerWidget::setAutoManageTextUrl(bool newValue)
{
    mUi->lText->setAutoManageUrl(newValue);
}

void BannerWidget::setLinkText(const QString& displayText)
{
    mUi->bLink->setText(displayText);
    mUi->wLinkContainer->show();
}

void BannerWidget::setTitle(const QString& text)
{
    mUi->lTitle->setText(text);
    mUi->lTitle->show();
}
