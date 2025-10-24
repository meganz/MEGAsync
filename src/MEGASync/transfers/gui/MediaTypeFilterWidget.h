#ifndef MEDIA_TYPE_FILTER_WIDGET_H
#define MEDIA_TYPE_FILTER_WIDGET_H

#include "TabSelector.h"
#include "TransfersWidget.h"

#include <QFrame>
#include <QWidget>

namespace Ui
{
class MediaTypeFilterWidget;
}

class MediaTypeFilterWidget: public QWidget
{
    Q_OBJECT

public:
    explicit MediaTypeFilterWidget(QWidget* parent = nullptr);
    ~MediaTypeFilterWidget();

    void resetCounter(TransfersWidget::TM_TAB tab);
    void setCounter(TransfersWidget::TM_TAB tab, unsigned long long counter);

    TabSelector* getTabSelectorByType(TransfersWidget::TM_TAB tab);
    TransfersWidget::TM_TAB getTabByTabSelector(TabSelector* tabSelector);

protected:
    bool event(QEvent* event) override;

private slots:
    void handleGroupboxToggled(bool checked);

private:
    Ui::MediaTypeFilterWidget* mUi;
    QMap<TransfersWidget::TM_TAB, TabSelector*> mTabSelectorsByType;

    void updateStrings();
};

#endif // MEDIA_TYPE_FILTER_WIDGET_H
