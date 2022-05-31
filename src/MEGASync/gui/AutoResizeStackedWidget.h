#ifndef AUTORESIZESTACKEDWIDGET_H
#define AUTORESIZESTACKEDWIDGET_H

#include <QStackedWidget>
#include <QEvent>

class AutoResizeStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    AutoResizeStackedWidget(QWidget* parent): QStackedWidget(parent){
        connect(this, &QStackedWidget::currentChanged, this, &AutoResizeStackedWidget::onUpdateHeight);
    }

    bool event(QEvent* e) override
    {
        if(e->type() == QEvent::Polish)
        {
            onUpdateHeight();
        }

        return QStackedWidget::event(e);
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if(watched == currentWidget() && event->type() == QEvent::Resize)
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

        setFixedHeight(currentWidget() ? currentWidget()->sizeHint().height() : minimumHeight());

        if(currentWidget())
        {
            currentWidget()->installEventFilter(this);
        }
    }
};

#endif // AUTORESIZESTACKEDWIDGET_H
