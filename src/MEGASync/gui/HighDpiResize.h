#ifndef HIGHDPIRESIZE_H
#define HIGHDPIRESIZE_H

#include <QDialog>

class HighDpiResize : public QObject
{
    Q_OBJECT

public:
    void init(QDialog* d);
    void queueRedraw();

private slots:
    void forceRedraw();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QDialog* dialog = nullptr;
};

#endif // HIGHDPIRESIZE_H
