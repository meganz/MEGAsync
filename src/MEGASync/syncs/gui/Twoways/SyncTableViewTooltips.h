#ifndef SYNCTABLEVIEWTOOLTIPS_H
#define SYNCTABLEVIEWTOOLTIPS_H

#include "syncs/model/SyncItemModel.h"

#include <QObject>
#include <QTableView>

class SyncTableViewTooltips : public QObject
{
public:
    SyncTableViewTooltips() = default;
    virtual ~SyncTableViewTooltips() = default;

    void setSourceModel(SyncItemModel* model);

protected:
    bool eventFilter(QObject *watched, QEvent *event);    
    static bool isInIcon(const QPoint& mousePos, int columnPosX);

    SyncItemModel* mModel;

private:
    virtual QString getTooltipText(const QPoint& mousePos, int columnPosX,
                           const QModelIndex& index);

    static QPoint correctedMousePosition(QTableView* viewDelegate,
                                         const QPoint& mousePos);
};

#endif // SYNCTABLEVIEWTOOLTIPS_H
