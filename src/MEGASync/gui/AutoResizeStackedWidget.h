#ifndef AUTORESIZESTACKEDWIDGET_H
#define AUTORESIZESTACKEDWIDGET_H

#include <QEvent>
#include <QResizeEvent>
#include <QStackedWidget>

class AutoResizeStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    AutoResizeStackedWidget(QWidget* parent): QStackedWidget(parent){
    }

    bool event(QEvent* e) override
    {
        if(e->type() == QEvent::Polish)
        {
            mMinimumHeight = minimumHeight();
            connect(this, &QStackedWidget::currentChanged, this, &AutoResizeStackedWidget::onUpdateHeight, Qt::UniqueConnection);
            onUpdateHeight();
        }

        return QStackedWidget::event(e);
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        //With the LayoutRequest event we get all the changes inside the widget, like hidding some widget inside it
        if(watched == currentWidget() && event->type() == QEvent::LayoutRequest)
        {
            onUpdateHeight();
        }

        return QStackedWidget::eventFilter(watched, event);
    }

    void resizeEvent(QResizeEvent* e) override
    {
        QStackedWidget::resizeEvent(e);

        mLaidOut = true;

        // The height depends on the current width (word-wrapping pages), so a width
        // change must recompute it; LayoutRequest alone does not fire on plain resizes.
        onUpdateHeight();
    }

private slots:
    void onUpdateHeight()
    {
        for(int index = 0; index < count(); ++index)
        {
            widget(index)->removeEventFilter(this);
        }

        auto currentHeight(mMinimumHeight);
        if (currentWidget())
        {
            // sizeHint() of pages with word-wrapping labels is computed for the labels'
            // transient widths, so it overshoots; heightForWidth() gives the height for
            // the real stack width. Before the first resizeEvent the width is a
            // placeholder, and a fixed height computed from it would force the window
            // to grow, so fall back to sizeHint() until then.
            currentHeight = (mLaidOut && currentWidget()->hasHeightForWidth() && width() > 0) ?
                                currentWidget()->heightForWidth(width()) :
                                currentWidget()->sizeHint().height();
        }
        setFixedHeight(currentHeight >= mMinimumHeight ? currentHeight : mMinimumHeight);

        if(currentWidget())
        {
            currentWidget()->installEventFilter(this);
        }
    }

private:
    int mMinimumHeight;
    bool mLaidOut = false;
};

#endif // AUTORESIZESTACKEDWIDGET_H
