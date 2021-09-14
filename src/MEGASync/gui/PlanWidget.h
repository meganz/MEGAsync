#ifndef PLANWIDGET_H
#define PLANWIDGET_H

#include "BalloonToolTip.h"
#include "Utilities.h"

#include <megaapi.h>

#include <QWidget>

namespace Ui {
class PlanWidget;
}

class PlanWidget : public QWidget
{
        Q_OBJECT

    public:
        typedef enum {
            FREE     = mega::MegaAccountDetails::ACCOUNT_TYPE_FREE,
            PRO_I    = mega::MegaAccountDetails::ACCOUNT_TYPE_PROI,
            PRO_II   = mega::MegaAccountDetails::ACCOUNT_TYPE_PROII,
            PRO_III  = mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII,
            PRO_LITE = mega::MegaAccountDetails::ACCOUNT_TYPE_LITE,
            BUSINESS = mega::MegaAccountDetails::ACCOUNT_TYPE_BUSINESS,
        } ProLevel;

        typedef enum {
            HELP      = 0,
            STORAGE   = 1,
            BANDWIDTH = 2,
        } HelpButton;

        explicit PlanWidget(const PlanInfo& data, const QString& userAgent, QWidget* parent);
        ~PlanWidget();

        void setPlanInfo(const PlanInfo& data);
        bool isBillingCurrency() const;
        int getPriceFontSizePx() const;
        void setPriceFontSizePx(int fontSizepx);

    protected:
        void changeEvent(QEvent* event) override;
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        Ui::PlanWidget* mUi;
        PlanInfo mDetails;
        QString mUserAgent;
        std::unique_ptr<BalloonToolTip> mTooltip;
        bool mDisabled;
        bool mIsBillingCurrency;

        void updatePlanInfo();
        void setWidgetOpacity(qreal opacity);
        QString getProURL();
        QString formatRichString(QString str, int type);
        QString getTooltipMsg(HelpButton hoverOver = HELP);
        QString toPrice (double value, const QString& currencySymbol);
};

#endif // PLANWIDGET_H
