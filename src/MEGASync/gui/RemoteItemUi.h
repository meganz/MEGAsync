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

    void setAddButtonEnabled(bool enabled);

signals:
    void addClicked(const QString& remoteFolder = QString());
    void deleteClicked();

#ifndef Q_OS_WIN
    void permissionsClicked();
#endif

protected:
    void changeEvent(QEvent* event) override;

private:
    void setTableViewProperties(QTableView* view) const;

    Ui::RemoteItemUi *ui;
};

#endif // REMOTEITEMUI_H
