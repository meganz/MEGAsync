#ifndef REMOTEITEMUI_H
#define REMOTEITEMUI_H

#include <QGroupBox>

namespace Ui {
class RemoteItemUi;
}

class RemoteItemUi : public QGroupBox
{
    Q_OBJECT

public:
    explicit RemoteItemUi(QWidget *parent = nullptr);
    ~RemoteItemUi();

    void setUsePermissions(const bool use);

private:
    Ui::RemoteItemUi *ui;
};

#endif // REMOTEITEMUI_H
