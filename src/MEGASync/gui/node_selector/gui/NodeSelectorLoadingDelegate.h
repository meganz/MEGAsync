#ifndef NODESELECTORLOADINGDELEGATE_H
#define NODESELECTORLOADINGDELEGATE_H

#include <QWidget>

namespace Ui {
class NodeSelectorLoadingDelegate;
}

class NodeSelectorLoadingDelegate : public QWidget
{
    Q_OBJECT

public:
    explicit NodeSelectorLoadingDelegate(QWidget *parent = nullptr);
    ~NodeSelectorLoadingDelegate();

    static QSize widgetSize();

private:
    Ui::NodeSelectorLoadingDelegate *ui;
};

#endif // NODESELECTORLOADINGDELEGATE_H
