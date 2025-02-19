#include "NodeSelector.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "QMegaMessageBox.h"
#include "ui_NodeSelector.h"
#include "Utilities.h"
#include "ViewLoadingScene.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

#include <optional>

using namespace mega;

const char* ITS_ON_NS = "itsOn";

NodeSelector::NodeSelector(SelectTypeSPtr selectType, QWidget* parent):
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    ui(new Ui::NodeSelector),
    mSelectType(selectType),
    mDelegateListener(std::make_unique<QTMegaListener>(mMegaApi, this)),
    mInitialised(false),
    mDuplicatedType(std::nullopt),
    mDuplicatedModel(nullptr)
{
    ui->setupUi(this);

    mMegaApi->addListener(mDelegateListener.get());

    connect(ui->bShowIncomingShares, &QPushButton::clicked, this, &NodeSelector::onbShowIncomingSharesClicked, Qt::QueuedConnection);
    connect(ui->bShowCloudDrive, &QPushButton::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->bShowBackups, &QPushButton::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
    connect(ui->bSearchNS, &QPushButton::clicked, this, &NodeSelector::onbShowSearchClicked);
    connect(ui->bRubbish, &QPushButton::clicked, this, &NodeSelector::onbShowRubbishClicked);

    foreach(auto& button, ui->wLeftPaneNS->findChildren<QAbstractButton*>())
    {
        button->installEventFilter(this);
        mButtonIconManager.addButton(button);
    }

    QColor shadowColor (188, 188, 188);
    mShadowTab = new QGraphicsDropShadowEffect(ui->buttonGroup);
    mShadowTab->setBlurRadius(10.);
    mShadowTab->setXOffset(0.);
    mShadowTab->setYOffset(0.);
    mShadowTab->setColor(shadowColor);
    mShadowTab->setEnabled(true);

    mTabFramesToggleGroup[SEARCH] = ui->fSearchStringNS;
    mTabFramesToggleGroup[BACKUPS] = ui->fBackups;
    mTabFramesToggleGroup[SHARES] = ui->fIncomingShares;
    mTabFramesToggleGroup[CLOUD_DRIVE] = ui->fCloudDrive;
    mTabFramesToggleGroup[RUBBISH] = ui->fRubbish;
    ui->wSearchNS->hide();
    ui->bSearchNS->hide();
    ui->fRubbish->hide();
    setAllFramesItsOnProperty();

    updateNodeSelectorTabs();
    onOptionSelected(CLOUD_DRIVE);
}

NodeSelector::~NodeSelector()
{
    // Remove duplicated node dialog if it is currently open
    if (mDuplicatedType.has_value())
    {
        DialogOpener::closeDialogsByClass<DuplicatedNodeDialog>();
    }
    mMegaApi->removeListener(mDelegateListener.get());
    delete ui;
}

void NodeSelector::setAllFramesItsOnProperty()
{
    for (auto tabFrame : qAsConst(mTabFramesToggleGroup))
    {
        tabFrame->setProperty(ITS_ON_NS, false);
    }
}

void NodeSelector::updateNodeSelectorTabs()
{
    ui->bShowCloudDrive->setText(MegaNodeNames::getCloudDriveName());
    ui->bShowIncomingShares->setText(MegaNodeNames::getIncomingSharesName());
    ui->bShowBackups->setText(MegaNodeNames::getBackupsName());
    ui->bRubbish->setText(MegaNodeNames::getRubbishName());
}

void NodeSelector::onSearch(const QString &text)
{
    ui->bSearchNS->setText(text);
    ui->wSearchNS->setVisible(true);
    ui->bSearchNS->setVisible(true);

    mSearchWidget->search(text);
    mSearchWidget->setSearchText(text);
    onbShowSearchClicked();
    ui->bSearchNS->setChecked(true);

    auto senderViewWidget = getTreeViewWidget(sender());
    if (senderViewWidget && senderViewWidget != mSearchWidget)
    {
        senderViewWidget->clearSearchText();
    }
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->showDefaultUploadOption(show);
        }
    }
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->setDefaultUploadOption(value);
        }
    }
}

bool NodeSelector::getDefaultUploadOption()
{
    return mCloudDriveWidget->getDefaultUploadOption();
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        updateNodeSelectorTabs();
    }
    QDialog::changeEvent(event);
}

void NodeSelector::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        e->ignore();
        return;
    }
    }

    QDialog::keyPressEvent(e);
}

void NodeSelector::mousePressEvent(QMouseEvent *event)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->clearSelection();
        }
    }

    QDialog::mousePressEvent(event);
}

void NodeSelector::showEvent(QShowEvent*)
{
    if(!mInitialised)
    {
        createSpecialisedWidgets();
        addSearch();
        initSpecialisedWidgets();

        mInitialised = true;
    }
}

void NodeSelector::onbOkClicked()
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer && viewContainer != getCurrentTreeViewWidget())
        {
            viewContainer->abort();
        }
    }

    onOkButtonClicked();
}

void NodeSelector::on_tClearSearchResultNS_clicked()
{
    ui->wSearchNS->hide();
    ui->bSearchNS->hide();
    ui->bSearchNS->setText(QString());
    mSearchWidget->stopSearch();
    if (getCurrentTreeViewWidget() == mSearchWidget)
    {
        onbShowCloudDriveClicked();
    }
}

void NodeSelector::onUpdateLoadingMessage(std::shared_ptr<MessageInfo> message)
{
    auto viewContainer = getCurrentTreeViewWidget();
    if (viewContainer && viewContainer->getProxyModel()->getMegaModel() == sender())
    {
        viewContainer->updateLoadingMessage(message);
    }
}

void NodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles,
                                         int extraUpdateNodesOnTarget,
                                         int)
{
    performItemsToBeMoved(handles,
                          extraUpdateNodesOnTarget,
                          IncreaseOrDecrease::INCREASE,
                          true,
                          true);
}

void NodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles,
                                               int extraUpdateNodesOnTarget,
                                               int)
{
    performItemsToBeMoved(handles,
                          extraUpdateNodesOnTarget,
                          IncreaseOrDecrease::DECREASE,
                          true,
                          true);
}

void NodeSelector::performItemsToBeMoved(const QList<mega::MegaHandle>& handles,
                                         int extraUpdateNodesOnTarget,
                                         IncreaseOrDecrease type,
                                         bool blockSource,
                                         bool blockTarget)
{
    if (handles.isEmpty())
    {
        return;
    }

    // IF we want to block the source or target, set values to false in order to look for them
    bool foundSource(blockSource ? false : true);
    bool foundTarget(blockTarget ? false : true);

    auto senderModel(dynamic_cast<NodeSelectorModel*>(sender()));

    auto targetOrSourceFound = [type](NodeSelectorTreeViewWidget* wid, int nodesUpdateToReceive)
    {
        if (wid)
        {
            if (type == IncreaseOrDecrease::INCREASE)
            {
                wid->initMovingNodes(nodesUpdateToReceive);
            }
            else
            {
                wid->decreaseMovingNodes(nodesUpdateToReceive);
            }
        }
    };

    NodeSelectorTreeViewWidget* sourceWid(nullptr);
    NodeSelectorTreeViewWidget* targetWid(nullptr);

    auto findSource = [handles, &foundSource, &sourceWid](NodeSelectorTreeViewWidget* wid)
    {
        if (!foundSource && wid->areItemsAboutToBeMovedFromHere(handles.first()))
        {
            foundSource = true;
            sourceWid = wid;
        }
    };

    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            if (!foundTarget && wid->getProxyModel()->getMegaModel() == senderModel)
            {
                foundTarget = true;
                targetWid = wid;

                findSource(wid);
            }
            else if (!foundSource)
            {
                findSource(wid);
            }

            if (foundSource && foundTarget)
            {
                break;
            }
        }
    }

    targetOrSourceFound(targetWid, handles.size() + extraUpdateNodesOnTarget);

    if (targetWid != sourceWid)
    {
        targetOrSourceFound(sourceWid, handles.size());
    }
}

void NodeSelector::onOptionSelected(int index)
{
    switch (index)
    {
        case NodeSelector::CLOUD_DRIVE:
            ui->bShowCloudDrive->click();
            break;
        case NodeSelector::SHARES:
            ui->bShowIncomingShares->click();
            break;
        case NodeSelector::BACKUPS:
            ui->bShowBackups->click();
            break;
        case NodeSelector::RUBBISH:
            ui->bRubbish->click();
            break;
        default:
            break;
    }
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
    setToggledStyle(CLOUD_DRIVE);
}

void NodeSelector::onbShowRubbishClicked()
{
    ui->stackedWidget->setCurrentIndex(RUBBISH);
    setToggledStyle(RUBBISH);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    if(ui->bShowIncomingShares->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(SHARES);
        setToggledStyle(SHARES);
    }
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    if(ui->bShowBackups->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(BACKUPS);
        setToggledStyle(BACKUPS);
    }
}

void NodeSelector::onbShowSearchClicked()
{
    if(ui->bSearchNS->isVisible())
    {
        ui->stackedWidget->setCurrentWidget(mSearchWidget);
        setToggledStyle(SEARCH);
    }
}

void NodeSelector::shortCutConnects(int ignoreThis)
{
    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i <= BACKUPS; ++i)
    {
        if(i != ignoreThis)
        {
            QShortcut *shortcut = new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i+1)), this);
            QObject::connect(shortcut, &QShortcut::activated, this, [=](){ onOptionSelected(i); });
        }
    }
}

NodeSelectorTreeViewWidget* NodeSelector::getTreeViewWidget(int page) const
{
    return dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
}

NodeSelectorTreeViewWidget* NodeSelector::getTreeViewWidget(QObject* object) const
{
    return dynamic_cast<NodeSelectorTreeViewWidget*>(object);
}

NodeSelectorTreeViewWidget* NodeSelector::getCurrentTreeViewWidget() const
{
    return getTreeViewWidget(ui->stackedWidget->currentWidget());
}

MegaHandle NodeSelector::getSelectedNodeHandle()
{
    auto tree_view = getCurrentTreeViewWidget();
    return tree_view ? tree_view->getSelectedNodeHandle() : mega::INVALID_HANDLE;
}

QList<MegaHandle> NodeSelector::getMultiSelectionNodeHandle()
{
    auto tree_view = getCurrentTreeViewWidget();
    return tree_view ? tree_view->getMultiSelectionNodeHandle() : QList<MegaHandle>();
}

void NodeSelector::closeEvent(QCloseEvent* event)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->abort();
            if(viewContainer->getProxyModel()->isModelProcessing())
            {
                connect(viewContainer->getProxyModel()->getMegaModel(), &NodeSelectorModel::blockUi, this, [this](bool blocked){
                    if(!blocked)
                    {
                        close();
                    }
                });
                event->ignore();
                return;
            }
        }
    }

    QDialog::closeEvent(event);
}

void NodeSelector::setToggledStyle(TabItem item)
{
    setAllFramesItsOnProperty();

    auto btn = mTabFramesToggleGroup[item]->findChildren<QPushButton*>();
    if(btn.size() > 0)
    {
        btn.at(0)->setChecked(true);
    }

    mTabFramesToggleGroup[item]->setProperty(ITS_ON_NS, true);
    mTabFramesToggleGroup[item]->setGraphicsEffect(mShadowTab);

    // Reload QSS because it is glitchy
    ui->wLeftPaneNS->setStyleSheet(ui->wLeftPaneNS->styleSheet());
}

std::shared_ptr<MegaNode> NodeSelector::getSelectedNode()
{
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    return node;
}

void NodeSelector::showNotFoundNodeMessageBox()
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = QMegaMessageBox::errorTitle();
    msgInfo.text = tr("The item you selected has been removed. To reselect, close this window and try again.");
    QMegaMessageBox::warning(msgInfo);
}

void NodeSelector::initSpecialisedWidgets()
{
    NodeSelectorModel* model(nullptr);

    auto selectedTab = selectedNodeTab();

    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if(viewContainer)
        {
            viewContainer->init();
            connect(viewContainer, &NodeSelectorTreeViewWidget::onCustomBottomButtonClicked, this, &NodeSelector::onCustomBottomButtonClicked, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::okBtnClicked, this, &NodeSelector::onbOkClicked, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::cancelBtnClicked, this, &NodeSelector::reject, Qt::UniqueConnection);
            connect(viewContainer,
                    &NodeSelectorTreeViewWidget::onSearch,
                    this,
                    &NodeSelector::onSearch,
                    Qt::UniqueConnection);

            if (selectedTab.has_value() && selectedTab.value() == page)
            {
                connect(viewContainer,
                        &NodeSelectorTreeViewWidget::viewReady,
                        this,
                        &NodeSelector::performNodeSelection);
            }

            model = viewContainer->getProxyModel()->getMegaModel();

            connect(model,
                    &NodeSelectorModel::itemsAboutToBeMoved,
                    this,
                    &NodeSelector::onItemsAboutToBeMoved);

            connect(model,
                    &NodeSelectorModel::itemsAboutToBeMovedFailed,
                    this,
                    &NodeSelector::onItemsAboutToBeMovedFailed);

            connect(model,
                    &NodeSelectorModel::mergeItemAboutToBeMoved,
                    this,
                    &NodeSelector::onMergeItemsAboutToBeMoved);

            connect(model,
                    &NodeSelectorModel::itemsAboutToBeRestored,
                    this,
                    &NodeSelector::onItemsAboutToBeRestored);

            connect(model, &NodeSelectorModel::mergeFinished, this, &NodeSelector::onMergeFinished);

            connect(model,
                    &NodeSelectorModel::updateLoadingMessage,
                    this,
                    &NodeSelector::onUpdateLoadingMessage);

            connect(model,
                    &NodeSelectorModel::showMessageBox,
                    this,
                    [this](QMegaMessageBox::MessageBoxInfo info)
                    {
                        info.parent = this;
                        QMegaMessageBox::warning(info);
                    });
            connect(model,
                    &NodeSelectorModel::showDuplicatedNodeDialog,
                    this,
                    [this, model](std::shared_ptr<ConflictTypes> conflicts,
                                  NodeSelectorModel::ActionType type)
                    {
                        mDuplicatedModel = model;
                        mDuplicatedConflicts = conflicts;
                        mDuplicatedType = type;

                        auto checkUploadNameDialog = new DuplicatedNodeDialog();
                        checkUploadNameDialog->setConflicts(conflicts);

                        DialogOpener::showDialog<DuplicatedNodeDialog, NodeSelector>(
                            checkUploadNameDialog,
                            this,
                            &NodeSelector::onShowDuplicatedNodeDialog);
                    });
        }
    }

    connect(ui->stackedWidget,
            &QStackedWidget::currentChanged,
            this,
            &NodeSelector::onCurrentWidgetChanged);
}

bool NodeSelector::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::DragEnter)
    {
        if(auto button = dynamic_cast<QPushButton*>(obj))
        {
            button->click();
        }
    }

    return QDialog::eventFilter(obj,event);
}

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node)
{
    mNodeToBeSelected = node;
    if (mInitialised)
    {
        performNodeSelection();
    }
}

void NodeSelector::performNodeSelection()
{
    if (mNodeToBeSelected)
    {
        std::optional<TabItem> option = selectedNodeTab();

        if (option.has_value())
        {
            auto optionValue(option.value());
            if (ui->stackedWidget->currentIndex() == optionValue)
            {
                onCurrentWidgetChanged(optionValue);
            }
            else
            {
                onOptionSelected(optionValue);
            }
        }
    }
}

std::optional<NodeSelector::TabItem> NodeSelector::selectedNodeTab()
{
    if (mNodeToBeSelected)
    {
        std::optional<TabItem> option;

        if (mMegaApi->isInCloud(mNodeToBeSelected.get()))
        {
            option = CLOUD_DRIVE;
        }
        else if (mMegaApi->isInVault(mNodeToBeSelected.get()))
        {
            option = BACKUPS;
        }
        else if (mMegaApi->isInRubbish(mNodeToBeSelected.get()))
        {
            option = RUBBISH;
        }
        else
        {
            option = SHARES;
        }

        if (option.has_value())
        {
            return option.value();
        }
    }

    return std::nullopt;
}

void NodeSelector::onCurrentWidgetChanged(int index)
{
    if (mNodeToBeSelected)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            wid->clearSelection();
            wid->setSelectedNodeHandle(mNodeToBeSelected->getHandle());
            mNodeToBeSelected.reset();
        }
    }
}

void NodeSelector::onShowDuplicatedNodeDialog(QPointer<DuplicatedNodeDialog>)
{
    if (mDuplicatedType.has_value())
    {
        mDuplicatedModel->processNodesAfterConflictCheck(
            mDuplicatedConflicts,
            static_cast<NodeSelectorModel::ActionType>(mDuplicatedType.value()));
        mDuplicatedType = std::nullopt;
        mDuplicatedModel = nullptr;
        mDuplicatedConflicts.reset();
    }
}

void NodeSelector::onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes)
{
    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            wid->onNodesUpdate(api, nodes);
        }
    }
}

void NodeSelector::addCloudDrive()
{
    mCloudDriveWidget = new NodeSelectorTreeViewWidgetCloudDrive(mSelectType);
    mCloudDriveWidget->setObjectName(QString::fromUtf8("CloudDrive"));
    ui->stackedWidget->addWidget(mCloudDriveWidget);
}

void NodeSelector::addIncomingShares()
{
    mIncomingSharesWidget = new NodeSelectorTreeViewWidgetIncomingShares(mSelectType);
    mIncomingSharesWidget->setObjectName(QString::fromUtf8("IncomingShares"));
    ui->stackedWidget->addWidget(mIncomingSharesWidget);
}

void NodeSelector::addBackups()
{
    mBackupsWidget = new NodeSelectorTreeViewWidgetBackups(mSelectType);
    mBackupsWidget->setObjectName(QString::fromUtf8("Backups"));
    ui->stackedWidget->addWidget(mBackupsWidget);
}

void NodeSelector::addSearch()
{
    mSearchWidget = new NodeSelectorTreeViewWidgetSearch(mSelectType);
    mSearchWidget->setObjectName(QString::fromUtf8("Search"));
    connect(mSearchWidget,
            &NodeSelectorTreeViewWidgetSearch::nodeDoubleClicked,
            this,
            &NodeSelector::setSelectedNodeHandle);
    ui->stackedWidget->addWidget(mSearchWidget);
}

void NodeSelector::addRubbish()
{
    mRubbishWidget = new NodeSelectorTreeViewWidgetRubbish(mSelectType);
    mRubbishWidget->setObjectName(QString::fromUtf8("Rubbish"));
    ui->stackedWidget->addWidget(mRubbishWidget);
}
