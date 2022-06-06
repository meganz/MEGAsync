#ifndef AUTORESIZESTACKEDWIDGET_H
#define AUTORESIZESTACKEDWIDGET_H

#include <QStackedWidget>
#include <QEvent>
#include <QDebug>

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
