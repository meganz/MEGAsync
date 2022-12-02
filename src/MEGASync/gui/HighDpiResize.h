#ifndef HIGHDPIRESIZE_H
#define HIGHDPIRESIZE_H

#include <QDialog>
#include <QPointer>

class HighDpiResize : public QObject
{
    Q_OBJECT

public:
    HighDpiResize() = default;
    HighDpiResize(QDialog* d);

    void init(QDialog* d);
    void queueRedraw();

private slots:
    void forceRedraw();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QPointer<QDialog> dialog = nullptr;
};

#endif // HIGHDPIRESIZE_H
