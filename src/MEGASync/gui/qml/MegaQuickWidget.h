#ifndef MEGA_QUICK_WIDGET_H
#define MEGA_QUICK_WIDGET_H

#include <QQuickWidget>
#include <QWidget>

class MegaQuickWidget: public QQuickWidget
{
    Q_OBJECT

public:
    explicit MegaQuickWidget(QWidget* parent = nullptr);
};
#endif
