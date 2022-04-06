#ifndef STALLEDISSUESVIEW_H
#define STALLEDISSUESVIEW_H

#include <QTreeView>

class StalledIssuesView : public QTreeView
{
    Q_OBJECT
public:
    StalledIssuesView(QWidget* parent);

    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;
};

#endif // STALLEDISSUESVIEW_H
