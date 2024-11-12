#ifndef VIEWLOADINGMESSAGE_H
#define VIEWLOADINGMESSAGE_H

#include <QWidget>

namespace Ui
{
class ViewLoadingMessage;
}

class ViewLoadingMessage : public QWidget
{
    Q_OBJECT

public:
    explicit ViewLoadingMessage(QWidget* parent = nullptr);
    ~ViewLoadingMessage();

private:
    Ui::ViewLoadingMessage* ui;
};

#endif // VIEWLOADINGMESSAGE_H
