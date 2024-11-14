#ifndef VIEWLOADINGMESSAGE_H
#define VIEWLOADINGMESSAGE_H

#include <QWidget>

namespace Ui
{
class ViewLoadingMessage;
}

struct MessageInfo;

class ViewLoadingMessage : public QWidget
{
    Q_OBJECT

public:
    explicit ViewLoadingMessage(QWidget* parent = nullptr);
    ~ViewLoadingMessage();

    void updateMessage(std::shared_ptr<MessageInfo> info);
    void updateGeometry();

    bool isButtonVisible() const;
    int getButtonType() const;

    void setCloseWhenAnyButtonIsPressed(bool newCloseWhenAnyButtonIsPressed);

signals:
    void buttonPressed(int);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onButtonPressed();

private:
    Ui::ViewLoadingMessage* ui;
    std::shared_ptr<MessageInfo> mInfo;
    bool mCloseWhenAnyButtonIsPressed;
};

#endif // VIEWLOADINGMESSAGE_H
