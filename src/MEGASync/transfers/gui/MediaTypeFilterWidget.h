#ifndef MEDIA_TYPE_FILTER_WIDGET_H
#define MEDIA_TYPE_FILTER_WIDGET_H

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

    QFrame* getFrame(TransfersWidget::TM_TAB tab) const;
    QLabel* getLabel(TransfersWidget::TM_TAB tab) const;

    void resetCounter(TransfersWidget::TM_TAB tab);
    void showIfGroupboxVisible(TransfersWidget::TM_TAB tab, unsigned long long counter);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void handleGroupboxToggled(bool checked);

private:
    Ui::MediaTypeFilterWidget* mUi;
    bool mIsOtherVisible = false;
    bool mIsAudioVisible = false;
    bool mIsVideoVisible = false;
    bool mIsArchiveVisible = false;
    bool mIsDocumentVisible = false;
    bool mIsImageVisible = false;
};

#endif // MEDIA_TYPE_FILTER_WIDGET_H
