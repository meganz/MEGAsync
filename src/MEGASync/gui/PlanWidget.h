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
            FREE          = mega::MegaAccountDetails::ACCOUNT_TYPE_FREE,
            PRO_I         = mega::MegaAccountDetails::ACCOUNT_TYPE_PROI,
            PRO_II        = mega::MegaAccountDetails::ACCOUNT_TYPE_PROII,
            PRO_III       = mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII,
            PRO_LITE      = mega::MegaAccountDetails::ACCOUNT_TYPE_LITE,
            PRO_STARTER   = mega::MegaAccountDetails::ACCOUNT_TYPE_STARTER,
            PRO_BASIC     = mega::MegaAccountDetails::ACCOUNT_TYPE_BASIC,
            PRO_ESSENTIAL = mega::MegaAccountDetails::ACCOUNT_TYPE_ESSENTIAL,
            BUSINESS      = mega::MegaAccountDetails::ACCOUNT_TYPE_BUSINESS,
            PRO_FLEXI     = mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI,
        } ProLevel;

        typedef enum {
            HELP      = 0,
            STORAGE   = 1,
            BANDWIDTH = 2,
        } HelpButton;

        explicit PlanWidget(const PlanInfo& data, const QString& userAgent, QWidget* parent);
        ~PlanWidget();

        void setPlanInfo(const PlanInfo& planData);
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

        //Map to fix order issues of Pro level list from SDK.
        std::map<int, int> visiblePlanOrder = {{FREE, 0},
                                               {PRO_STARTER, 1},
                                               {PRO_BASIC, 2},
                                               {PRO_ESSENTIAL, 3},
                                               {PRO_LITE, 4},
                                               {PRO_I, 5},
                                               {PRO_II, 6},
                                               {PRO_III, 7},
                                               {BUSINESS, 8},
                                               {PRO_FLEXI, 9}                                               };

        void updatePlanInfo();
        void setWidgetOpacity(qreal opacity);
        QString getProURL();
        QString getTooltipMsg(HelpButton hoverOver = HELP);
        QString toPrice (double value, const QString& currencySymbol);
};

#endif // PLANWIDGET_H
