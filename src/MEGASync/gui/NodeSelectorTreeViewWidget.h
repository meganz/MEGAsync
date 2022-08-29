#ifndef NODESELECTORTREEVIEWWIDGET_H
#define NODESELECTORTREEVIEWWIDGET_H

#include <QWidget>
#include <QItemSelectionModel>
#include <megaapi.h>>

namespace Ui {
class NodeSelectorTreeViewWidget;
}

class NodeSelectorTreeViewWidget : public QWidget
{
    Q_OBJECT

public:
    enum Type{
        UPLOAD_SELECT = 0,
        DOWNLOAD_SELECT,
        SYNC_SELECT,
        STREAM_SELECT,
    };

    explicit NodeSelectorTreeViewWidget(QWidget *parent = nullptr);
    ~NodeSelectorTreeViewWidget();

private slots:
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onDeleteClicked();
    void onGenMEGALinkClicked();
    void onItemDoubleClick(const QModelIndex &index);
    void onGoForwardClicked();
    void onGoBackClicked();
    void onSectionResized();


private:
    Ui::NodeSelectorTreeViewWidget *ui;
    int mSelectMode;
    mega::MegaApi* mMegaApi;
};

#endif // NODESELECTORTREEVIEWWIDGET_H
