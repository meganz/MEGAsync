#ifndef AUTORESIZESTACKEDWIDGET_H
#define AUTORESIZESTACKEDWIDGET_H

#include <QStackedWidget>
#include <QEvent>

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

private slots:
    void onUpdateHeight()
    {
        for(int index = 0; index < count(); ++index)
        {
            widget(index)->removeEventFilter(this);
        }

        auto currentHeight = currentWidget() ? currentWidget()->sizeHint().height() : mMinimumHeight;
        setFixedHeight(currentHeight >= mMinimumHeight ? currentHeight : mMinimumHeight);

        if(currentWidget())
        {
            currentWidget()->installEventFilter(this);
        }
    }

private:
    int mMinimumHeight;
};

#endif // AUTORESIZESTACKEDWIDGET_H
