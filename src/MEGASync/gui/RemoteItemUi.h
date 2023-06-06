#ifndef REMOTEITEMUI_H
#define REMOTEITEMUI_H

#include <QGroupBox>
#include <QTableView>

#include <megaapi.h>

namespace Ui {
class RemoteItemUi;
}

class RemoteItemUi : public QGroupBox
{
    Q_OBJECT

public:
    explicit RemoteItemUi(QWidget *parent = nullptr);
    ~RemoteItemUi();

    void initView(QTableView* newView);
    void setUsePermissions(const bool use);

    QTableView* getView();

signals:
    void addClicked(mega::MegaHandle);
    void deleteClicked();
    void permissionsClicked();

private:
    void setTableViewProperties(QTableView* view) const;

    Ui::RemoteItemUi *ui;
};

#endif // REMOTEITEMUI_H
