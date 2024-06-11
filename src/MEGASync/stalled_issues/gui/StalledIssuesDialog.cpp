#include "StalledIssuesDialog.h"
#include "ui_StalledIssuesDialog.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueDelegate.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssue.h"

#include "Utilities.h"

const char* MODE_SELECTED = "SELECTED";

StalledIssuesDialog::StalledIssuesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StalledIssuesDialog),
    mCurrentTab(StalledIssueFilterCriterion::ALL_ISSUES),
    mProxyModel(nullptr),
    mDelegate(nullptr)
{
    ui->setupUi(this);
#ifndef Q_OS_MACOS
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#endif

    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::uiBlocked,
            this,  &StalledIssuesDialog::onUiBlocked);
    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::uiUnblocked,
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

    mProxyModel = new StalledIssuesProxyModel(this);
    mProxyModel->setSourceModel(MegaSyncApp->getStalledIssuesModel());
    connect(mProxyModel, &StalledIssuesProxyModel::modelFiltered, this, &StalledIssuesDialog::onModelFiltered);

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::updateLoadingMessage, ui->stalledIssuesTree->getLoadingMessageHandler(), &LoadingSceneMessageHandler::updateMessage, Qt::QueuedConnection);
    connect(ui->stalledIssuesTree->getLoadingMessageHandler(), &LoadingSceneMessageHandler::onStopPressed, this, [this](){
        MegaSyncApp->getStalledIssuesModel()->stopSolvingIssues();
    });

    mDelegate = new StalledIssueDelegate(mProxyModel, ui->stalledIssuesTree);
    ui->stalledIssuesTree->setItemDelegate(mDelegate);
    connect(&ui->stalledIssuesTree->loadingView(), &ViewLoadingSceneBase::sceneVisibilityChange, this, &StalledIssuesDialog::onLoadingSceneVisibilityChange);

    connect(ui->SettingsButton, &QPushButton::clicked, this, [](){
        MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
    });

    connect(ui->HelpButton, &QPushButton::clicked, this, [](){
        Utilities::openUrl(QUrl(Utilities::SYNC_SUPPORT_URL));
    });

    showView();
    if(MegaSyncApp->getStalledIssuesModel()->issuesRequested())
    {
        onUiBlocked();
    }
}

StalledIssuesDialog::~StalledIssuesDialog()
{
    delete ui;
}

QModelIndexList StalledIssuesDialog::getSelection(QList<mega::MegaSyncStall::SyncStallReason> reasons) const
{
    auto checkerFunc = [reasons](const std::shared_ptr<const StalledIssue> check) -> bool{
        return reasons.contains(check->getReason());
    };

    return getSelection(checkerFunc);
}

QModelIndexList StalledIssuesDialog::getSelection(std::function<bool (const std::shared_ptr<const StalledIssue>)> checker) const
{
    QModelIndexList list;

    auto selectedIndexes = ui->stalledIssuesTree->selectionModel()->selectedIndexes();
    foreach(auto index, selectedIndexes)
    {
        //Just in case, but children is never selected
        if(!index.parent().isValid())
        {
            QModelIndex sourceIndex = mProxyModel->mapToSource(index);
            if(sourceIndex.isValid())
            {
                auto stalledIssueItem (qvariant_cast<StalledIssueVariant>(sourceIndex.data(Qt::DisplayRole)));
                if(!stalledIssueItem.consultData()->isSolved() && checker(stalledIssueItem.consultData()))
                {
                    list.append(sourceIndex);
                }
            }
        }
    }

    return list;
}

void StalledIssuesDialog::mouseReleaseEvent(QMouseEvent *event)
{
    //User cliked outside the view
    ui->stalledIssuesTree->clearSelection();

    QDialog::mouseReleaseEvent(event);
}

void StalledIssuesDialog::on_doneButton_clicked()
{
    close();
}

void StalledIssuesDialog::on_refreshButton_clicked()
{
    mProxyModel->updateStalledIssues();
}

void StalledIssuesDialog::checkIfViewIsEmpty()
{
    if(auto proxyModel = dynamic_cast<StalledIssuesProxyModel*>(ui->stalledIssuesTree->model()))
    {
        auto isEmpty = proxyModel->rowCount(QModelIndex()) == 0;
        ui->TreeViewContainer->setCurrentWidget(isEmpty ? ui->EmptyViewContainerPage : ui->TreeViewContainerPage);
    }
}

void StalledIssuesDialog::toggleTab(StalledIssueFilterCriterion filterCriterion)
{
  if(auto proxyModel = dynamic_cast<StalledIssuesProxyModel*>(ui->stalledIssuesTree->model()))
  {
      //Show the view to show the loading view
      ui->TreeViewContainer->setCurrentWidget(ui->TreeViewContainerPage);
      proxyModel->filter(filterCriterion);
  }
}

void StalledIssuesDialog::onUiBlocked()
{
    if(!ui->stalledIssuesTree->loadingView().isLoadingViewSet())
    {
        ui->TreeViewContainer->setCurrentWidget(ui->TreeViewContainerPage);
        ui->stalledIssuesTree->loadingView().toggleLoadingScene(true);
    }
}

void StalledIssuesDialog::onUiUnblocked()
{
    if(ui->stalledIssuesTree->loadingView().isLoadingViewSet())
    {
        ui->stalledIssuesTree->loadingView().toggleLoadingScene(false);
    }
}

void StalledIssuesDialog::onStalledIssuesLoaded()
{
    mDelegate->resetCache();
    mProxyModel->updateFilter();
}

void StalledIssuesDialog::onModelFiltered()
{
    //Only the first time, in order to avoid setting the model before it is sorted
    if(!ui->stalledIssuesTree->model())
    {
        ui->stalledIssuesTree->setModel(mProxyModel);
        mViewHoverManager.setView(ui->stalledIssuesTree);
    }

    checkIfViewIsEmpty();
}

void StalledIssuesDialog::onLoadingSceneVisibilityChange(bool state)
{
    ui->footer->setDisabled(state);
    ui->header->setDisabled(state);
}

void StalledIssuesDialog::showView()
{
    ui->stackedWidget->setCurrentWidget(ui->View);
    on_refreshButton_clicked();
}

void StalledIssuesDialog::onGlobalSyncStateChanged(bool)
{
    //For the future, detect if the stalled issues have been removed remotely to close the dialog
}

void StalledIssuesDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
