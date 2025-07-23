#ifndef REFRESHAPPCHANGEEVENT_H
#define REFRESHAPPCHANGEEVENT_H

#include <QApplication>
#include <QEvent>
#include <QWidget>

class RefreshAppChangeEvent
{
public:
    inline static QEvent::Type ThemeChanged =
        static_cast<QEvent::Type>(QEvent::registerEventType());

    static bool isRefreshEvent(QEvent* event)
    {
        return event->type() == ThemeChanged || event->type() == QEvent::LanguageChange;
    }

    static void propagateRefreshEvent()
    {
        auto event(new QEvent(ThemeChanged));

        const auto widgets = QApplication::allWidgets();
        for (QWidget* widget: widgets)
        {
            QApplication::sendEvent(widget, event);
        }

        delete event;
    }

private:
    RefreshAppChangeEvent() = default;
};

#endif // REFRESHAPPCHANGEEVENT_H
