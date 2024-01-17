#include "PlanWidget.h"
#include "ui_PlanWidget.h"
#include "Utilities.h"
#include "Preferences/Preferences.h"
#include "megaapi.h"
#include "MegaApplication.h"

#include <QtConcurrent/QtConcurrent>
#include <QUrl>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>

constexpr int NB_B_IN_1GB (1024 * 1024 * 1024);
constexpr int NB_GB_IN_1TB (1024);

PlanWidget::PlanWidget(const PlanInfo& data, const QString& userAgent, QWidget* parent) :
    QWidget (parent),
    mUi (new Ui::PlanWidget),
    mDetails (data),
    mUserAgent (userAgent),
    mTooltip (new BalloonToolTip(this)),
    mDisabled (false),
    mIsBillingCurrency (data.billingCurrencyName == data.localCurrencyName)
{
    mUi->setupUi(this);
    setMouseTracking(true);    

    updatePlanInfo();

    installEventFilter(this);
    mUi->lHelp->installEventFilter(this);
    mUi->lBusinessStorageIcon->installEventFilter(this);
    mUi->lBusinessTransferIcon->installEventFilter(this);        
}

PlanWidget::~PlanWidget()
{
    delete mUi;
}

void PlanWidget::updatePlanInfo()
{
    auto accountType (mDetails.level);
    mDisabled = false;
    mIsBillingCurrency = mDetails.billingCurrencyName == mDetails.localCurrencyName;

    // Set widget opacity for lower plans than the actual one
    int currentAccType (Preferences::instance()->accountType());
    if (currentAccType != FREE && currentAccType != PRO_LITE)
    {
        if (mDetails.level == PRO_LITE || mDetails.level < currentAccType)
        {
            setWidgetOpacity(0.5);
            mDisabled = true;
        }
    }

    // Set css props for different pro plans
    setCursor(!mDisabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
    setProperty("disabled", mDisabled);
    setProperty("currentPlan", mDetails.level == currentAccType);

    // Draw colored border and tag if this is the current plan
    if (mDetails.level == currentAccType)
    {
        QLabel* tag = new QLabel(tr("Current plan"), this);
        tag->setObjectName(QString::fromLatin1("lCurrentPlanTag"));
        tag->adjustSize();
        tag->move((width() - tag->width()) / 2,
                  mUi->wContainer->y()
                  + mUi->wContainer->layout()->contentsMargins().top()
                  - 2 // border size
                  - tag->height() / 2);
    }

    // Display currency name only if the local currency is not the billing currency
    mUi->lCurrency->setVisible(!mIsBillingCurrency);

    // Set prices strings
    double localPrice ((mDetails.pricePerUserLocal * mDetails.minUsers) / 100.);
    QString localPriceString (toPrice(localPrice, mDetails.localCurrencySymbol));
    mUi->lPrice->setText(localPriceString);
    mUi->lCurrency->setText(mDetails.localCurrencyName);

    // Reset lPrice StyleSheet
    mUi->lPrice->setStyleSheet(QString());
    // Force polish to update font Info with .ui StyleSheet
    mUi->lPrice->style()->polish(mUi->lPrice);

    // If text does not fit, use a smaller font size
    QFont font (mUi->lPrice->font());
    auto fontSize (font.pixelSize());
    while (QFontMetrics(font).boundingRect(localPriceString).width() > mUi->lPrice->width()
           && fontSize > 1)
    {
        fontSize -= 1;
        font.setPixelSize(fontSize);
    }
    setPriceFontSizePx(fontSize);

    QString smallPrice;
    if (accountType == BUSINESS)
    {
        QString price (toPrice(mDetails.pricePerUserBilling / 100.,
                               mDetails.billingCurrencySymbol));
        smallPrice = tr("%1 per user").arg(price);
        mUi->lMinimumUsersBusiness->setText(tr("minimum %n user", "", mDetails.minUsers));
    }
    else
    {
        double billingPrice ((mDetails.pricePerUserBilling * mDetails.minUsers) / 100.);
        smallPrice = toPrice(billingPrice, mDetails.billingCurrencySymbol);
    }
    mUi->lPriceBusiness->setText(smallPrice);

    // Set period
    mUi->lPeriod->setText(QString::fromUtf8("/%1").arg(tr("month")));

    switch (mDetails.level)
    {
        case PRO_LITE:
            mUi->lProPlan->setText(QString::fromUtf8("Pro Lite"));
            break;
        case PRO_I:
            mUi->lProPlan->setText(QString::fromUtf8("Pro I"));
            break;
        case PRO_II:
            mUi->lProPlan->setText(QString::fromUtf8("Pro II"));
            break;
        case PRO_III:
            mUi->lProPlan->setText(QString::fromUtf8("Pro III"));
            break;
        case BUSINESS:
            mUi->lProPlan->setText(QString::fromUtf8("Business"));
            break;
        default:
            mUi->lProPlan->setText(QString::fromUtf8("Pro"));
            break;
    }

    // Show/hide widgets according to plans/currency
    mUi->lGreatValue->setVisible(accountType == PRO_I);
    mUi->lPriceBusiness->setVisible(accountType == BUSINESS || !mIsBillingCurrency);
    mUi->lMinimumUsersBusiness->setVisible(accountType == BUSINESS);
    mUi->lPeriod->setAlignment(Qt::AlignVCenter |
                               ((accountType == BUSINESS)
                                || !mIsBillingCurrency ?
                                    Qt::AlignRight
                                  : Qt::AlignLeft));

    // Choose the right icon to show: check mark or ?
    if (accountType == BUSINESS)
    {
        mUi->sIconsStorage->setCurrentWidget(mUi->pBusinessTierStorage);
        mUi->sIconsTransfer->setCurrentWidget(mUi->pBusinessTierTransfer);
    }
    else
    {
        if (mDetails.level == currentAccType)
        {
            mUi->sIconsStorage->setCurrentWidget(mUi->pProTierStorageCurrentPlan);
            mUi->sIconsTransfer->setCurrentWidget(mUi->pProTierTransferCurrentPlan);
        }
        else
        {
            mUi->sIconsStorage->setCurrentWidget(mUi->pProTierStorage);
            mUi->sIconsTransfer->setCurrentWidget(mUi->pProTierTransfer);
        }
    }

    //Set limits
    mUi->lStorageInfo->setText(tr("[A] storage").replace(QString::fromLatin1("[A]"),
                                                         Utilities::getSizeString(mDetails.gbStorage * NB_B_IN_1GB)));
    mUi->lBandWidthInfo->setText(tr("[A] transfer").replace(QString::fromLatin1("[A]"),
                                                            Utilities::getSizeString(mDetails.gbTransfer * NB_B_IN_1GB)));

    style()->unpolish(this);
    style()->polish(this);
}

void PlanWidget::setPlanInfo(const PlanInfo& planData)
{
    mDetails = planData;
    updatePlanInfo();
}

bool PlanWidget::isBillingCurrency() const
{
    return mIsBillingCurrency;
}

int PlanWidget::getPriceFontSizePx() const
{
    // Force polish to update font Info
    mUi->lPrice->style()->polish(mUi->lPrice);
    return mUi->lPrice->fontInfo().pixelSize();
}

void PlanWidget::setPriceFontSizePx(int fontSizepx)
{
    // Force polish to update font Info
    if (mUi->lPrice->font().pixelSize() != fontSizepx)
    {
        // Use StyleSheet because it overrides setFont()
        mUi->lPrice->setStyleSheet(QString::fromLatin1("QLabel{font-size: %1px;}").arg(fontSizepx));
    }
}

void PlanWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updatePlanInfo();
    }
    QWidget::changeEvent(event);
}

bool PlanWidget::eventFilter(QObject* obj, QEvent* event)
{
    // If pro card is not disabled, custom management of events, otherwise do nothing
    if (!mDisabled)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QString url = getProURL();
            Utilities::getPROurlWithParameters(url);
            Utilities::openUrl(QUrl(url));

            return true;
        }
        else if (event->type() == QEvent::ToolTip)
        {
            bool hoverHelpIcon {obj == mUi->lHelp};
            bool hoverStorageInfo {obj == mUi->lBusinessStorageIcon};
            bool hoverTransferInfo {obj == mUi->lBusinessTransferIcon};

            if (hoverHelpIcon || hoverStorageInfo || hoverTransferInfo)
            {
                auto target (static_cast<QWidget*>(obj));

                HelpButton ho (HELP);
                if (hoverStorageInfo)
                {
                    ho = STORAGE;
                }
                else if (hoverTransferInfo)
                {
                    ho = BANDWIDTH;
                }

                mTooltip->setPopupText(getTooltipMsg(ho));

                // Compute tooltip's attach point (in target coords ref)
                QPoint attachPoint (target->width() / 2, 0);
                mTooltip->attachAt(target->mapToGlobal(attachPoint));
                mTooltip->show();
            }
            else
            {
                mTooltip.get()->hide();
                event->ignore();
            }

            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

QString PlanWidget::getTooltipMsg(HelpButton hoverOver)
{
    QString msg;
    switch (hoverOver)
    {
        case HELP:
        {
            switch (mDetails.level)
            {
                case PRO_I:
                {
                    msg = tr("Great value for secure cloud storage, sharing and communication.");
                    break;
                }
                case PRO_II:
                {
                    msg = tr("Rest easy knowing you have plenty of secure cloud storage.");
                    break;
                }
                case PRO_III:
                {
                    msg = tr("Store even more with our premium secure cloud storage offering.");
                    break;
                }
                case BUSINESS:
                {
                    msg = tr("With our end-to-end encryption, the way your team works has never been more secure and private.");
                    break;
                }
                case PRO_LITE:
                default:
                {
                    msg = tr("Get started with secure file storage.");
                    break;
                }
            }
            break;
        }
        case STORAGE:
        {
            int tbPerTransfer (mDetails.gbPerStorage / NB_GB_IN_1TB);
            QString price (toPrice(mDetails.pricePerTransferLocal / 100.,
                                   mDetails.localCurrencySymbol));
            msg = tr("Additional storage charged at %1 per %2TB.")
                  .arg(price, tbPerTransfer > 1 ?
                           QLocale::system().toString(tbPerTransfer) + QChar(QChar::Nbsp)
                         : QString());
            break;
        }
        case BANDWIDTH:
        {
            int tbPerStorage (mDetails.gbPerTransfer / NB_GB_IN_1TB);
            QString price (toPrice(mDetails.pricePerStorageLocal / 100.,
                                   mDetails.localCurrencySymbol));
            msg = tr("Additional transfer quota charged at %1 per %2TB.")
                  .arg(price, tbPerStorage > 1 ?
                           QLocale::system().toString(tbPerStorage) + QChar(QChar::Nbsp)
                         : QString());
            break;
        }
    }
    return msg;
}

void PlanWidget::setWidgetOpacity(qreal opacity)
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(opacity);
    setGraphicsEffect(effect);
}

QString PlanWidget::getProURL()
{
    QString url;
    switch (mDetails.level)
    {
        case PRO_LITE:
            url = QString::fromUtf8("mega://#propay_4");
            break;
        case PRO_I:
            url = QString::fromUtf8("mega://#propay_1");
            break;
        case PRO_II:
            url = QString::fromUtf8("mega://#propay_2");
            break;
        case PRO_III:
            url = QString::fromUtf8("mega://#propay_3");
            break;
        case BUSINESS:
            url = QString::fromUtf8("mega://#registerb");
            break;
        default:
            url = QString::fromUtf8("mega://#pro");
            break;
    }

    return url;
}

QString PlanWidget::toPrice(double value, const QString& currencySymbol)
{
    // Build locale: it is necessary to build it like this because using QLocale::system()
    // ignores the precision when using toCurrencyString.
    static const QLocale locale (QLocale().language(), QLocale().country());
    int precision (std::fmod(value, 1.) > 0. ? 2 : 0);

    QString price (locale.toCurrencyString(value, currencySymbol, precision));
    if (!mIsBillingCurrency && currencySymbol != mDetails.billingCurrencySymbol)
    {
        price += QLatin1Char('*');
    }
    return price;
}
