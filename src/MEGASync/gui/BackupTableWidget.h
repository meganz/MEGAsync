#ifndef BACKUPTABLEWIDGET_H
#define BACKUPTABLEWIDGET_H

#include <QObject>
#include <QTableView>

class BackupTableWidget : public QTableView
{
    Q_OBJECT

public:
    BackupTableWidget(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCustomContextMenuRequested(const QPoint& pos);
    void onCellClicked(const QModelIndex &index);

private:
    void showContextMenu(const QPoint &pos, const QModelIndex index);
};

#endif // BACKUPTABLEWIDGET_H
