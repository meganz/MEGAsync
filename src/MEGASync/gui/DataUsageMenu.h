#ifndef DATAUSAGEMENU_H
#define DATAUSAGEMENU_H

#include <QMenu>

class DataUsageMenu : public QMenu
{
    Q_OBJECT

public:
    explicit DataUsageMenu(QWidget *parent = 0);
    ~DataUsageMenu();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPolygonF polygon;
};

#endif // DATAUSAGEMENU_H
