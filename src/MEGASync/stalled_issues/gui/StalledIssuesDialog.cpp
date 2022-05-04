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
    mProxyModel(nullptr),
    mCurrentTab(StalledIssueFilterCriterion::ALL_ISSUES)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    mProxyModel = new StalledIssuesProxyModel(this);
    mProxyModel->setSourceModel(MegaSyncApp->getStalledIssuesModel());

    ui->stalledIssuesTree->setModel(mProxyModel);
    mViewHoverManager.setView(ui->stalledIssuesTree);

    auto delegate = new StalledIssueDelegate(mProxyModel, ui->stalledIssuesTree);
    ui->stalledIssuesTree->setItemDelegate(delegate);
    mLoadingScene.setView(ui->stalledIssuesTree);

    connect(mProxyModel, &StalledIssuesProxyModel::uiBlocked,
            this,  &StalledIssuesDialog::onUiBlocked);
    connect(mProxyModel, &StalledIssuesProxyModel::uiUnblocked,
            this,  &StalledIssuesDialog::onUiUnblocked);

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::stalledIssuesReceived,
            this,  &StalledIssuesDialog::onStalledIssuesLoaded);

    //Init all categories
    auto tabs = ui->header->findChildren<StalledIssueTab*>();
    foreach(auto tab, tabs)
    {
        connect(tab, &StalledIssueTab::tabToggled, this, &StalledIssuesDialog::toggleTab);
    }

    ui->allIssuesTab->setItsOn(true);

    on_updateButton_clicked();
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
    onUiBlocked();
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

void StalledIssuesDialog::onUiBlocked()
{
    setDisabled(true);
    mLoadingScene.setLoadingScene(true);
}

void StalledIssuesDialog::onUiUnblocked()
{
    setDisabled(false);
    mLoadingScene.setLoadingScene(false);
}

void StalledIssuesDialog::onStalledIssuesLoaded()
{
    onUiUnblocked();
    mProxyModel->updateFilter();
}
