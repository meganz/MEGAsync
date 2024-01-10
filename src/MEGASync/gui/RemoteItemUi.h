#ifndef REMOTEITEMUI_H
#define REMOTEITEMUI_H

#include <QWidget>
#include <QTableView>

#include <megaapi.h>

namespace Ui {
class RemoteItemUi;
}

class RemoteItemUi : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteItemUi(QWidget *parent = nullptr);
    ~RemoteItemUi();

    void setTitle(const QString& title);
    void initView(QTableView* newView);
    void setUsePermissions(const bool use);

    QTableView* getView();

signals:
    void addClicked(mega::MegaHandle);
    void deleteClicked();

#ifndef Q_OS_WIN
    void permissionsClicked();
#endif

private:
    void setTableViewProperties(QTableView* view) const;

    Ui::RemoteItemUi *ui;
};

#endif // REMOTEITEMUI_H
