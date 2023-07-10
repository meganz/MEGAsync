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

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::globalSyncStateChanged,
            this,  &StalledIssuesDialog::onGlobalSyncStateChanged);

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

    mDelegate = new StalledIssueDelegate(mProxyModel, ui->stalledIssuesTree);
    ui->stalledIssuesTree->setItemDelegate(mDelegate);
    connect(&ui->stalledIssuesTree->loadingView(), &ViewLoadingSceneBase::sceneVisibilityChange, this, &StalledIssuesDialog::onLoadingSceneChanged);

    connect(ui->ModeSelectorButton, &QPushButton::clicked, this, &StalledIssuesDialog::showModeSelector);

    connect(ui->SelectButton, &QPushButton::clicked, this, [this](){
        auto valueChanged(mModeSelected != Preferences::instance()->stalledIssueMode());
        if(valueChanged)
        {
            Preferences::instance()->setStalledIssueMode(mModeSelected);
        }
        showView(valueChanged);
    });

    if(Preferences::instance()->stalledIssueMode() == Preferences::StalledIssuesModeType::None)
    {
        showModeSelector();
    }
    else
    {
        showView(true);
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
                if(checker(stalledIssueItem.consultData()))
                {
                    list.append(sourceIndex);
                }
            }
        }
    }

    return list;
}

void StalledIssuesDialog::updateView()
{
    ui->stalledIssuesTree->update();
}

bool StalledIssuesDialog::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        if(obj == ui->Verbose)
        {
            ui->Smart->setProperty(MODE_SELECTED, false);
            ui->Smart->setStyleSheet(ui->Smart->styleSheet());
            ui->Verbose->setProperty(MODE_SELECTED, true);
            ui->Verbose->setStyleSheet(ui->Verbose->styleSheet());

            mModeSelected = Preferences::StalledIssuesModeType::Verbose;
        }
        else if(obj == ui->Smart)
        {
            ui->Smart->setProperty(MODE_SELECTED, true);
            ui->Smart->setStyleSheet(ui->Smart->styleSheet());
            ui->Verbose->setProperty(MODE_SELECTED, false);
            ui->Verbose->setStyleSheet(ui->Verbose->styleSheet());

            mModeSelected = Preferences::StalledIssuesModeType::Smart;
        }

        ui->SelectButton->setEnabled(true);
    }

    return QDialog::eventFilter(obj, event);
}

void StalledIssuesDialog::on_doneButton_clicked()
{
    close();
}

void StalledIssuesDialog::on_updateButton_clicked()
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

void StalledIssuesDialog::onLoadingSceneChanged(bool state)
{
    setDisabled(state);
}

void StalledIssuesDialog::showModeSelector()
{
    auto mode = Preferences::instance()->stalledIssueMode();
    if(mode != Preferences::StalledIssuesModeType::None)
    {
        if(mode == Preferences::StalledIssuesModeType::Smart)
        {
            ui->Smart->setProperty(MODE_SELECTED, true);
            ui->Verbose->setProperty(MODE_SELECTED, false);
        }
        else
        {
            ui->Smart->setProperty(MODE_SELECTED, false);
            ui->Verbose->setProperty(MODE_SELECTED, true);
        }

        ui->SelectButton->setEnabled(true);
    }
    else
    {
        ui->Smart->setProperty(MODE_SELECTED, false);
        ui->Verbose->setProperty(MODE_SELECTED, false);
    }

    ui->Smart->setStyleSheet(ui->Smart->styleSheet());
    ui->Verbose->setStyleSheet(ui->Verbose->styleSheet());

    ui->stackedWidget->setCurrentWidget(ui->ModeSelector);
    ui->Verbose->installEventFilter(this);
    ui->Smart->installEventFilter(this);
}

void StalledIssuesDialog::showView(bool update)
{
    ui->stackedWidget->setCurrentWidget(ui->View);

    if(update)
    {
        on_updateButton_clicked();
    }
}

void StalledIssuesDialog::onGlobalSyncStateChanged(bool)
{
    //For the future, detect if the stalled issues have been removed remotely to close the dialog
}
