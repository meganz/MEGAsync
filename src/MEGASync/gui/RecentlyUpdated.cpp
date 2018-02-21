#include "RecentlyUpdated.h"
#include "ui_RecentlyUpdated.h"
#include "MegaApplication.h"

RecentlyUpdated::RecentlyUpdated(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecentlyUpdated)
{
    ui->setupUi(this);
    ui->wTransfers->setupFinishedTransfers(((MegaApplication *)qApp)->getFinishedTransfers(), QTransfersModel::TYPE_RECENTLY_UPDATED);

    setVisualMode(COLLAPSED);
}

RecentlyUpdated::~RecentlyUpdated()
{
    delete ui;
}

void RecentlyUpdated::setVisualMode(int mode)
{
    actualMode = mode;
    switch(actualMode)
    {
        case EXPANDED: //Show header and tree view with list of updated transfers
            setMaximumHeight(191);
            ui->wTransfers->show();
            break;
        case COLLAPSED: //Show only Recently updated header
            ui->wTransfers->hide();
            setMaximumHeight(31);
            break;
    }
}

void RecentlyUpdated::onTransferFinish(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError *e)
{
    ui->wTransfers->getModel()->onTransferFinish(api, transfer, e);
}

int RecentlyUpdated::getActualMode() const
{
    return actualMode;
}

void RecentlyUpdated::on_cRecentlyUpdated_stateChanged(int arg1)
{
    switch (arg1)
    {
        case Qt::Checked:
            emit onRecentlyUpdatedClicked(EXPANDED);
            break;
        case Qt::Unchecked:
            default:
            emit onRecentlyUpdatedClicked(COLLAPSED);
            break;
    }
}
