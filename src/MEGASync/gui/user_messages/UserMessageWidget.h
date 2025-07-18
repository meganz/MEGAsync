#ifndef USER_MESSAGE_WIDGET_H
#define USER_MESSAGE_WIDGET_H

#include "ThemeManager.h"
#include "TokenParserWidgetManager.h"
#include "UserMessage.h"

#include <QWidget>

class UserMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserMessageWidget(QWidget* parent = nullptr):
        QWidget(parent)
    {}
    virtual ~UserMessageWidget() = default;

    virtual void setData(UserMessage* data) = 0;
    virtual UserMessage* getData() const = 0;

    virtual bool needThemeUpdate() const
    {
        return mCurrentTheme != ThemeManager::instance()->getSelectedTheme();
    }

    virtual void applyTheme()
    {
        if (needThemeUpdate())
        {
            TokenParserWidgetManager::instance()->applyCurrentTheme(this, true);
            mCurrentTheme = ThemeManager::instance()->getSelectedTheme();
            setData(getData());
        }
    }

signals:
    void dataChanged();

private:
    Preferences::ThemeType mCurrentTheme = Preferences::ThemeType::LAST;
};

#endif // USER_MESSAGE_WIDGET_H
