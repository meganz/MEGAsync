#ifndef REFRESHAPPCHANGEEVENT_H
#define REFRESHAPPCHANGEEVENT_H

#include <QApplication>
#include <QEvent>
#include <QWidget>

constexpr QEvent::Type AppRefreshEvent = QEvent::LanguageChange /*QEvent::ModifiedChange*/;

class RefreshAppChangeEvent: public QWidget
{
public:
    static bool isRefreshEvent(QEvent* event)
    {
        return event->type() == QEvent::ModifiedChange || event->type() == QEvent::LanguageChange;
    }

    static void propagateRefreshEvent()
    {
        auto event(new QEvent(AppRefreshEvent));

        const auto topLevelWidgets = QApplication::topLevelWidgets();
        for (QWidget* widget: topLevelWidgets)
        {
            QApplication::sendEvent(widget, event);
        }

        delete event;
    }

private:
    friend class ThemeManager;
    RefreshAppChangeEvent() = default;
};

#endif // REFRESHAPPCHANGEEVENT_H
