#include "StalledIssuesDialog.h"
#include "ui_StalledIssuesDialog.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueDelegate.h"
#include "StalledIssuesProxyModel.h"

#include "Utilities.h"

StalledIssuesDialog::StalledIssuesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StalledIssuesDialog),
    mCurrentTab(StalledIssueFilterCriterion::ALL_ISSUES)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    auto model = new StalledIssuesProxyModel(this);
    model->setSourceModel(MegaSyncApp->getStalledIssuesModel());

    ui->stalledIssuesTree->setModel(model);
    mViewHoverManager.setView(ui->stalledIssuesTree);

    auto delegate = new StalledIssueDelegate(model, ui->stalledIssuesTree);
    ui->stalledIssuesTree->setItemDelegate(delegate);

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::stalledIssuesCountChanged,
            this,  &StalledIssuesDialog::onStalledIssuesModelCountChanged);

    MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();

    //Init all categories
    auto tabs = ui->header->findChildren<StalledIssueTab*>();
    foreach(auto tab, tabs)
    {
        connect(tab, &StalledIssueTab::tabToggled, this, &StalledIssuesDialog::toggleTab);
    }

    ui->allIssuesTab->setItsOn(true);
}

StalledIssuesDialog::~StalledIssuesDialog()
{
    delete ui;
}

bool StalledIssuesDialog::eventFilter(QObject* obj, QEvent* event)
{
    return QDialog::eventFilter(obj, event);
}

void StalledIssuesDialog::on_doneButton_clicked()
{
    close();
}

void StalledIssuesDialog::on_updateButton_clicked()
{
    MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
}

void StalledIssuesDialog::onStalledIssuesModelCountChanged()
{
    auto isEmpty = ui->stalledIssuesTree->model()->rowCount(QModelIndex()) == 0;
    ui->TreeViewContainer->setCurrentWidget(isEmpty ? ui->EmptyViewContainerPage : ui->TreeViewContainerPage);
}

void StalledIssuesDialog::toggleTab(StalledIssueFilterCriterion filterCriterion)
{
  if(auto proxyModel = dynamic_cast<StalledIssuesProxyModel*>(ui->stalledIssuesTree->model()))
  {
      proxyModel->filter(filterCriterion);
      onStalledIssuesModelCountChanged();
  }
}
