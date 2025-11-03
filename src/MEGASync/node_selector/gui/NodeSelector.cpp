#include "NodeSelector.h"

#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MessageDialogOpener.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "TabSelector.h"
#include "TokenizableItems/TokenPropertySetter.h"
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
    mDuplicatedModel(nullptr),
    mTargetWid(nullptr)
{
    ui->setupUi(this);
    ui->cbAlwaysUploadToLocation->hide();

    connect(ui->stackedWidget,
            &QStackedWidget::currentChanged,
            this,
            &NodeSelector::onCurrentWidgetChanged);

    mMegaApi->addListener(mDelegateListener.get());

    connect(ui->fIncomingShares,
            &TabSelector::clicked,
            this,
            &NodeSelector::onbShowIncomingSharesClicked,
            Qt::QueuedConnection);
    connect(ui->fCloudDrive, &TabSelector::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->fBackups, &TabSelector::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
    connect(ui->fRubbish, &TabSelector::clicked, this, &NodeSelector::onbShowRubbishClicked);
    connect(ui->fSearch, &TabSelector::clicked, this, &NodeSelector::onbShowSearchClicked);

    connect(ui->fSearch, &TabSelector::hidden, this, &NodeSelector::onfShowSearchHidden);
    connect(ui->leSearch, &SearchLineEdit::search, this, &NodeSelector::onSearch);

    ui->leSearch->addCustomWidget(ui->wTitleContainer);

    ui->wSearch->hide();
    ui->fRubbish->hide();

    updateNodeSelectorTabs();
    onOptionSelected(CLOUD_DRIVE);

    // Left pane tokens
    {
        BaseTokens iconTokens;
        iconTokens.setNormalOff(QLatin1String("icon-secondary"));
        iconTokens.setNormalOn(QLatin1String("icon-primary"));
        auto iconTokenSetter = std::make_shared<TokenPropertySetter>(iconTokens);

        TabSelector::applyTokens(ui->wLeftPaneNS, iconTokenSetter);
    }

    if (!mSelectType->footerVisible())
    {
        ui->footer->hide();
        ui->wRightPaneNS->layout()->setContentsMargins(0, 0, 0, 14);
    }

    resetButtonsText();

    ui->bOk->setDefault(true);
    ui->bOk->setEnabled(false);

    connect(ui->bOk, &QPushButton::clicked, this, &NodeSelector::onbOkClicked);
    connect(ui->bCancel, &QPushButton::clicked, this, &NodeSelector::reject);

    resize(1024, 720);
    setMinimumSize(760, 400);
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

void NodeSelector::init()
{
    createSpecialisedWidgets();
    addSearch();
    initSpecialisedWidgets();

    mInitialised = true;
}

void NodeSelector::updateNodeSelectorTabs()
{
    ui->fCloudDrive->setTitle(MegaNodeNames::getCloudDriveName());
    ui->fIncomingShares->setTitle(MegaNodeNames::getIncomingSharesName());
    ui->fBackups->setTitle(MegaNodeNames::getBackupsName());
    ui->fRubbish->setTitle(MegaNodeNames::getRubbishName());
}

void NodeSelector::onSearch(const QString& text)
{
    ui->wSearch->show();
    ui->fSearch->setTitle(text);
    ui->fSearch->setSelected(true);

    mSearchWidget->search(text);
    onbShowSearchClicked();
}

void NodeSelector::onUiIsBlocked(bool state)
{
    ui->bCancel->setDisabled(state);
    if (state)
    {
        ui->bOk->setDisabled(true);
    }
}

void NodeSelector::onSelectionChanged(bool state)
{
    ui->bOk->setEnabled(state);
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    ui->cbAlwaysUploadToLocation->setVisible(show);
}

void NodeSelector::setDefaultUploadOption(bool value)
{
    ui->cbAlwaysUploadToLocation->setChecked(value);
}

bool NodeSelector::getDefaultUploadOption()
{
    return ui->cbAlwaysUploadToLocation->isChecked();
}

bool NodeSelector::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        resetButtonsText();
        updateNodeSelectorTabs();
        onLanguageChangeEvent();
    }

    return QDialog::event(event);
}

void NodeSelector::mousePressEvent(QMouseEvent* event)
{
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->clearSelection();
        }
    }

    QDialog::mousePressEvent(event);
}

void NodeSelector::onbOkClicked()
{
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer && viewContainer != getCurrentTreeViewWidget())
        {
            viewContainer->abort();
        }
    }

    onOkButtonClicked();
}

void NodeSelector::onfShowSearchHidden()
{
    ui->wSearch->hide();
    ui->fSearch->setTitle(QString());
    ui->leSearch->onClearClicked();
    mSearchWidget->stopSearch();
    if (getCurrentTreeViewWidget() == mSearchWidget)
    {
        onOptionSelected(CLOUD_DRIVE);
    }
}

void NodeSelector::onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::INCREASE, true, true);
}

void NodeSelector::onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int)
{
    performItemsToBeMoved(handles, IncreaseOrDecrease::DECREASE, true, true);
}

void NodeSelector::performItemsToBeMoved(const QList<mega::MegaHandle>& handles,
                                         IncreaseOrDecrease type,
                                         bool blockSource,
                                         bool blockTarget)
{
    if (handles.isEmpty())
    {
        return;
    }

    mTargetWid = nullptr;
    mSourceWids.clear();

    // IF we want to block the source or target, set values to false in order to look for them
    bool foundSource(!blockSource);
    bool foundTarget(!blockTarget);

    auto senderModel(dynamic_cast<NodeSelectorModel*>(sender()));

    auto targetOrSourceFound = [type](NodeSelectorTreeViewWidget* wid, int nodesUpdateToReceive)
    {
        if (wid)
        {
            if (type == IncreaseOrDecrease::INCREASE)
            {
                wid->increaseMovingNodes(nodesUpdateToReceive);
            }
            else
            {
                wid->decreaseMovingNodes(nodesUpdateToReceive);
            }
        }
    };

    auto findSource = [this, handles](NodeSelectorTreeViewWidget* wid)
    {
        if (wid->areItemsAboutToBeMovedFromHere(handles.first()))
        {
            mSourceWids.append(wid);
        }
    };

    for (int index = 0; index < ui->stackedWidget->count(); ++index)
    {
        if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
        {
            if (!foundTarget && wid->getProxyModel()->getMegaModel() == senderModel)
            {
                foundTarget = true;
                mTargetWid = wid;

                findSource(wid);
            }
            else if (!foundSource)
            {
                findSource(wid);
            }
        }
    }

    targetOrSourceFound(mTargetWid, handles.size());

    for (auto& wid: mSourceWids)
    {
        if (wid != mTargetWid)
        {
            targetOrSourceFound(wid, handles.size());
        }
    }
}

void NodeSelector::onOptionSelected(int index)
{
    switch (index)
    {
        case NodeSelector::CLOUD_DRIVE:
            ui->fCloudDrive->setSelected(true);
            break;
        case NodeSelector::SHARES:
            ui->fIncomingShares->setSelected(true);
            break;
        case NodeSelector::BACKUPS:
            ui->fBackups->setSelected(true);
            break;
        case NodeSelector::RUBBISH:
            ui->fRubbish->setSelected(true);
            break;
        case NodeSelector::SEARCH:
            ui->fSearch->setSelected(true);
            break;
        default:
            break;
    }
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
}

void NodeSelector::onbShowRubbishClicked()
{
    ui->stackedWidget->setCurrentIndex(RUBBISH);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    if (ui->fIncomingShares->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(SHARES);
    }
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    if (ui->fBackups->isVisible())
    {
        ui->stackedWidget->setCurrentIndex(BACKUPS);
    }
}

void NodeSelector::onbShowSearchClicked()
{
    if (ui->fSearch->isVisible())
    {
        ui->stackedWidget->setCurrentWidget(mSearchWidget);
    }
}

void NodeSelector::shortCutConnects(int ignoreThis)
{
    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i <= BACKUPS; ++i)
    {
        if (i != ignoreThis)
        {
            QShortcut* shortcut =
                new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i + 1)), this);
            QObject::connect(shortcut,
                             &QShortcut::activated,
                             this,
                             [=]()
                             {
                                 onOptionSelected(i);
                             });
        }
    }
}

void NodeSelector::resetButtonsText()
{
    // Done here to re-use contexts
    ui->bOk->setText(QCoreApplication::translate("NodeSelectorTreeViewWidget", "Ok"));
    ui->bCancel->setText(QCoreApplication::translate("NodeSelectorTreeViewWidget", "Cancel"));
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
    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->abort();
            if (viewContainer->getProxyModel()->isModelProcessing())
            {
                connect(viewContainer->getProxyModel()->getMegaModel(),
                        &NodeSelectorModel::blockUi,
                        this,
                        [this](bool blocked)
                        {
                            if (!blocked)
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

std::shared_ptr<MegaNode> NodeSelector::getSelectedNode()
{
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    return node;
}

void NodeSelector::showNotFoundNodeMessageBox()
{
    MessageDialogInfo msgInfo;
    msgInfo.descriptionText =
        tr("The item you selected has been removed. To reselect, close this window and try again.");
    MessageDialogOpener::warning(msgInfo);
}

void NodeSelector::initSpecialisedWidgets()
{
    NodeSelectorModel* model(nullptr);

    for (int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = getTreeViewWidget(page);
        if (viewContainer)
        {
            viewContainer->init();
            connect(viewContainer,
                    &NodeSelectorTreeViewWidget::onCustomButtonClicked,
                    this,
                    &NodeSelector::onCustomButtonClicked,
                    Qt::UniqueConnection);
            connect(viewContainer,
                    &NodeSelectorTreeViewWidget::uiIsBlocked,
                    this,
                    &NodeSelector::onUiIsBlocked,
                    Qt::UniqueConnection);
            connect(viewContainer,
                    &NodeSelectorTreeViewWidget::okBtnClicked,
                    this,
                    &NodeSelector::onbOkClicked,
                    Qt::UniqueConnection);
            connect(viewContainer,
                    &NodeSelectorTreeViewWidget::viewReady,
                    this,
                    &NodeSelector::performNodeSelection);

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
                    &NodeSelectorModel::itemsAboutToBeRestored,
                    this,
                    &NodeSelector::onItemsAboutToBeRestored);

            connect(model,
                    &NodeSelectorModel::itemAboutToBeReplaced,
                    this,
                    &NodeSelector::onItemAboutToBeReplaced);

            connect(model,
                    &NodeSelectorModel::itemsAboutToBeMerged,
                    this,
                    &NodeSelector::onItemsAboutToBeMerged);

            connect(model,
                    &NodeSelectorModel::itemsAboutToBeMergedFailed,
                    this,
                    &NodeSelector::onItemsAboutToBeMergedFailed);

            connect(model,
                    &NodeSelectorModel::showMessageBox,
                    this,
                    [this](MessageDialogInfo info)
                    {
                        info.parent = this;
                        MessageDialogOpener::warning(info);
                    });

            connect(model,
                    &NodeSelectorModel::showDuplicatedNodeDialog,
                    this,
                    [this, model](std::shared_ptr<ConflictTypes> conflicts, MoveActionType type)
                    {
                        mDuplicatedModel = model;
                        mDuplicatedConflicts = conflicts;
                        mDuplicatedType = type;

                        auto checkUploadNameDialog = new DuplicatedNodeDialog(this);
                        checkUploadNameDialog->setConflicts(conflicts);

                        DialogOpener::showDialog<DuplicatedNodeDialog, NodeSelector>(
                            checkUploadNameDialog,
                            this,
                            &NodeSelector::onShowDuplicatedNodeDialog);
                    });
        }
    }
}

bool NodeSelector::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::DragEnter)
    {
        if (auto button = dynamic_cast<QPushButton*>(obj))
        {
            button->click();
        }
    }

    return QDialog::eventFilter(obj, event);
}

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node)
{
    mNodeToBeSelected = node;
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

        // Disconnect all connections as soon as the node was selected
        if (!mNodeToBeSelected)
        {
            for (int index = 0; index < ui->stackedWidget->count(); ++index)
            {
                if (auto wid =
                        dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
                {
                    disconnect(wid,
                               &NodeSelectorTreeViewWidget::viewReady,
                               this,
                               &NodeSelector::performNodeSelection);
                }
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
    if (auto wid = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(index)))
    {
        if (mNodeToBeSelected)
        {
            wid->clearSelection();
            wid->setSelectedNodeHandle(mNodeToBeSelected->getHandle());
            mNodeToBeSelected.reset();
        }

        mSelectType->selectionHasChanged(wid);

        disconnect(mSelectionChangedConnection);
        mSelectionChangedConnection = connect(wid,
                                              &NodeSelectorTreeViewWidget::selectionIsCorrect,
                                              this,
                                              &NodeSelector::onSelectionChanged,
                                              Qt::UniqueConnection);

        onSelectionChanged(wid->isSelectionCorrect());
    }
}

void NodeSelector::onShowDuplicatedNodeDialog(QPointer<DuplicatedNodeDialog>)
{
    if (mDuplicatedType.has_value())
    {
        mDuplicatedModel->processNodesAfterConflictCheck(
            mDuplicatedConflicts,
            static_cast<MoveActionType>(mDuplicatedType.value()));
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
            if (wid != mSearchWidget || !ui->fSearch->isHidden())
            {
                wid->onNodesUpdate(api, nodes);
            }
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
            [this](std::shared_ptr<MegaNode> node)
            {
                setSelectedNodeHandle(node);
                performNodeSelection();
            });
    ui->stackedWidget->addWidget(mSearchWidget);
}

void NodeSelector::addRubbish()
{
    mRubbishWidget = new NodeSelectorTreeViewWidgetRubbish(mSelectType);
    mRubbishWidget->setObjectName(QString::fromUtf8("Rubbish"));
    ui->stackedWidget->addWidget(mRubbishWidget);
}
