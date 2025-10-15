#ifndef BANNER_WIDGET_H
#define BANNER_WIDGET_H

#include <QUrl>
#include <QWidget>

namespace Ui
{
class BannerWidget;
}

class BannerWidget: public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        NONE = 0,
        BANNER_WARNING = 1,
        BANNER_ERROR = 2,
        BANNER_INFO = 3
    };

    explicit BannerWidget(QWidget* parent = nullptr);
    ~BannerWidget();

    void setType(Type type);
    void setDescription(const QString& text);
    void setTitle(const QString& text);
    void setAutoManageTextUrl(bool newValue);
    void setLinkText(const QString& displayText);
signals:
    void linkActivated();

private:
    Ui::BannerWidget* mUi;
    Type mType;
};

#endif // BANNER_WIDGET_H
