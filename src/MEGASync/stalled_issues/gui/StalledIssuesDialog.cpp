#include "StalledIssuesDialog.h"
#include "ui_StalledIssuesDialog.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueDelegate.h"

StalledIssuesDialog::StalledIssuesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StalledIssuesDialog)
{
    ui->setupUi(this);

    auto model = new StalledIssuesProxyModel(this);
    model->setSourceModel(MegaSyncApp->getStalledIssuesModel());

    ui->stalledIssuesTree->setModel(model);
    mViewHoverManager.setView(ui->stalledIssuesTree);

    auto delegate = new StalledIssueDelegate(model, ui->stalledIssuesTree);
    ui->stalledIssuesTree->setItemDelegate(delegate);

    setAttribute(Qt::WA_DeleteOnClose, true);

    MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
}

StalledIssuesDialog::~StalledIssuesDialog()
{
    delete ui;
}

void StalledIssuesDialog::on_doneButton_clicked()
{
    close();
}

void StalledIssuesDialog::on_updateButton_clicked()
{
    MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
}
