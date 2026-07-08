#ifndef INCOMINGSHAREHEADERWIDGET_H
#define INCOMINGSHAREHEADERWIDGET_H

#include <QLabel>
#include <QPixmap>
#include <QWidget>

namespace Ui
{
class IncomingShareHeaderWidget;
}

struct IncomingShareHeaderData
{
    QString folderName;
    QPixmap userIcon;
    QString userName;
    QString userEmail;
    QPixmap accessIcon;
    QString accessLabel;
    int accessType = -1;
};

class IncomingShareHeaderWidget: public QWidget
{
    Q_OBJECT

public:
    explicit IncomingShareHeaderWidget(QWidget* parent = nullptr);
    ~IncomingShareHeaderWidget();

    void setData(const IncomingShareHeaderData& data);
    void clear();

signals:
    void menuRequested(const QPoint& globalPos);

private:
    Ui::IncomingShareHeaderWidget* ui = nullptr;
};

#endif // INCOMINGSHAREHEADERWIDGET_H
