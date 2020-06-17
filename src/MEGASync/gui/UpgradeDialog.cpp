#include "UpgradeDialog.h"
#include "ui_UpgradeDialog.h"
#include "Utilities.h"
#include "Preferences.h"
#include <QDateTime>
#include <QUrl>
#include "HighDpiResize.h"
#include <QMouseEvent>
#include "mega/types.h"

using namespace mega;

UpgradeDialog::UpgradeDialog(MegaApi *megaApi, MegaPricing *pricing, QWidget *parent)
    :QDialog(parent),
    ui(new Ui::UpgradeDialog),
    mPopOver{mega::make_unique<BandwidthOverquotaPopOver>(this)}
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->megaApi = megaApi;
    if (pricing)
    {
        this->pricing = pricing->copy();
    }
    else
    {
        this->pricing = NULL;
    }

    finishTime = 0;

    ui->labelInfo->setTextFormat(Qt::RichText);
    ui->labelInfo->setText(QString::fromUtf8("<p style=\"line-height: 20px;\">") + ui->labelInfo->text() + QString::fromUtf8("</p>"));
    refreshAccountDetails();

    plansLayout = new QHBoxLayout();
    plansLayout->setContentsMargins(18,0,18,0);
    plansLayout->setSpacing(0);

    delete ui->wPlans->layout();
    ui->wPlans->setLayout(plansLayout);

    updatePlans();

    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(unitTimeElapsed()));
    highDpiResize.init(this);
}

void UpgradeDialog::setTimestamp(long long time)
{
    finishTime = time;
    unitTimeElapsed();
    if (!timer->isActive())
    {
        timer->start(1000);
    }
}

void UpgradeDialog::setPricing(MegaPricing *pricing)
{
    if (!pricing)
    {
        return;
    }

    if (this->pricing)
    {
        delete this->pricing;
    }

    this->pricing = pricing->copy();
    updatePlans();
}

void UpgradeDialog::refreshAccountDetails()
{
    Preferences *preferences = Preferences::instance();
    if (preferences->isTemporalBandwidthValid() && preferences->temporalBandwidth())
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);

        mPopOver->updateMessage(tr("The IP address you are using has utilised %1 of data transfer in the last 6 hours,"
                                   " which took you over our current limit. To remove this limit,"
                                   " you can [A]upgrade to PRO[/A], which will give you your own transfer quota"
                                   " package and also ample extra storage space. ")
                           .arg(Utilities::getSizeString(preferences->temporalBandwidth()))
                           .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"%1\"><span style=\"color:#d90007; text-decoration:none;\">").arg(url))
                           .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>"))
                           .replace(QString::fromUtf8(" 6 "), QString::fromUtf8(" ").append(QString::number(preferences->temporalBandwidthInterval())).append(QString::fromUtf8(" ")))
                           + QString::fromUtf8("</p>"));
        ui->toolButtonQuestion->setVisible(true);
    }
    else
    {
        ui->toolButtonQuestion->setVisible(false);
    }
}

UpgradeDialog::~UpgradeDialog()
{
    delete ui;
    delete pricing;
}

void UpgradeDialog::updatePlans()
{
    if (!pricing)
    {
        return;
    }

    clearPlans();

    QString userAgent = QString::fromUtf8(megaApi->getUserAgent());
    int products = pricing->getNumProducts();
    for (int it = 0; it < products; it++)
    {
        if (pricing->getMonths(it) == 1)
        {
            PlanInfo data = {
                pricing->getAmount(it),
                convertCurrency(pricing->getCurrency(it)),
                pricing->getGBStorage(it),
                pricing->getGBTransfer(it),
                pricing->getProLevel(it)
            };
            plansLayout->addWidget(new PlanWidget(data, userAgent));
        }
    } 
}

void UpgradeDialog::clearPlans()
{
    while (QLayoutItem* item = plansLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            delete widget;
        }
        delete item;
    }
}

void UpgradeDialog::mousePressEvent(QMouseEvent *event)
{
    const auto mousePositionButtonRelated{ui->toolButtonQuestion->mapFrom(this, event->pos())};
    if (ui->toolButtonQuestion->rect().contains(mousePositionButtonRelated))
    {
#ifdef __APPLE__
        showPopOverRelativeToRect(winId(), m_popover, event->localPos());
#else

        const auto mouseGlobalPosition{event->globalPos()};

        mPopOver->show();
        mPopOver->ensurePolished();
        mPopOver->move(mouseGlobalPosition - QPoint(mPopOver->width()/2, mPopOver->height() + 10));
        Utilities::adjustToScreenFunc(mouseGlobalPosition, mPopOver.get());

        const auto initialWidth{mPopOver->width()};
        const auto initialHeight{mPopOver->height()};

        // size might be incorrect the first time it's shown. This works around that and repositions at the expected position afterwards
        QTimer::singleShot(1, this, [this, mouseGlobalPosition, initialWidth, initialHeight] ()
        {
            mPopOver->update();
            mPopOver->ensurePolished();

            const auto sizeChanged{initialWidth != mPopOver->width() || initialHeight != mPopOver->height()};
            if (sizeChanged)
            {
                mPopOver->move(mouseGlobalPosition - QPoint(mPopOver->width()/2, mPopOver->height()));
                Utilities::adjustToScreenFunc(mouseGlobalPosition, mPopOver.get());
                mPopOver->update();
            }
        });
#endif
    }
}

QString UpgradeDialog::convertCurrency(const char *currency)
{
    if (!strcmp(currency, "EUR"))
    {
        return QString::fromUtf8("€");
    }

    if (!strcmp(currency, "USD"))
    {
        return QString::fromUtf8("$");
    }

    return QString::fromUtf8(currency);
}

void UpgradeDialog::unitTimeElapsed()
{
    long long remainingTime = finishTime - QDateTime::currentMSecsSinceEpoch() / 1000;
    if (remainingTime < 0)
    {
        remainingTime = 0;
    }
    ui->labelRemainingTime->setText(tr("Please upgrade to Pro to continue immediately, or wait %1 to continue for free. ").arg(Utilities::getTimeString(remainingTime)));
}

void UpgradeDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if (!ui->labelInfo->text().contains(QString::fromUtf8("line-height")))
        {
            ui->labelInfo->setText(QString::fromUtf8("<p style=\"line-height: 20px;\">") + ui->labelInfo->text() + QString::fromUtf8("</p>"));
        }
        refreshAccountDetails();
    }
    QDialog::changeEvent(event);
}
