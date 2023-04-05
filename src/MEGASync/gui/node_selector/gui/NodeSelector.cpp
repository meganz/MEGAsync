#include "NodeSelector.h"
#include "ui_NodeSelector.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"
#include "megaapi.h"
#include "../model/NodeSelectorProxyModel.h"
#include "../model/NodeSelectorModel.h"
#include "NodeSelectorTreeViewWidgetSpecializations.h"
#include "NodeSelectorSpecializations.h"

#include "MegaNodeNames.h"

#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

using namespace mega;

const char* ITS_ON_NS = "itsOn";

NodeSelector::NodeSelector(QWidget *parent) :
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    ui(new Ui::NodeSelector)
{
    ui->setupUi(this);

    connect(ui->bShowIncomingShares, &QPushButton::clicked, this, &NodeSelector::onbShowIncomingSharesClicked);
    connect(ui->bShowCloudDrive, &QPushButton::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->bShowBackups, &QPushButton::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
    connect(ui->bSearch, &QPushButton::clicked, this, &NodeSelector::onbShowSearchClicked);

    foreach(auto& button, ui->wLeftPane->findChildren<QAbstractButton*>())
    {
        mButtonIconManager.addButton(button);
    }

    QColor shadowColor (188, 188, 188);
    mShadowTab = new QGraphicsDropShadowEffect(ui->buttonGroup);
    mShadowTab->setBlurRadius(10.);
    mShadowTab->setXOffset(0.);
    mShadowTab->setYOffset(0.);
    mShadowTab->setColor(shadowColor);
    mShadowTab->setEnabled(true);

    mTabFramesToggleGroup[SEARCH] = ui->fSearchString;
    mTabFramesToggleGroup[BACKUPS] = ui->fBackups;
    mTabFramesToggleGroup[SHARES] = ui->fIncomingShares;
    mTabFramesToggleGroup[CLOUD_DRIVE] = ui->fCloudDrive;
    ui->wSearch->hide();
    ui->bSearch->hide();
    setAllFramesItsOnProperty();

    updateNodeSelectorTabs();
    onOptionSelected(CLOUD_DRIVE);
}

NodeSelector::~NodeSelector()
{
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
}

void NodeSelector::onSearch(const QString &text)
{
    ui->bSearch->setText(text);
    ui->wSearch->setVisible(true);
    ui->bSearch->setVisible(true);

    mSearchWidget->search(text);
    mSearchWidget->setSearchText(text);
    onbShowSearchClicked();
    ui->bSearch->setChecked(true);

    auto senderViewWidget = dynamic_cast<NodeSelectorTreeViewWidget*>(sender());
    if(senderViewWidget != mSearchWidget)
    {
        senderViewWidget->clearSearchText();
    }
}

void NodeSelector::showDefaultUploadOption(bool show)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
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
        auto viewContainer = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
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
    }
    }
}

void NodeSelector::onbOkClicked()
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
        if(viewContainer && viewContainer != ui->stackedWidget->currentWidget())
        {
            viewContainer->abort();
        }
    }

    isSelectionCorrect() ? accept() : reject();
}

void NodeSelector::on_tClearSearchResult_clicked()
{
    ui->wSearch->hide();
    ui->bSearch->hide();
    ui->bSearch->setText(QString());
    mSearchWidget->stopSearch();
    if(ui->stackedWidget->currentWidget() == mSearchWidget)
    {
        onbShowCloudDriveClicked();
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
        default:
            break;
    }
}

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
    setToggledStyle(CLOUD_DRIVE);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    ui->stackedWidget->setCurrentIndex(SHARES);
    setToggledStyle(SHARES);
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    ui->stackedWidget->setCurrentIndex(BACKUPS);
    setToggledStyle(BACKUPS);
}

void NodeSelector::onbShowSearchClicked()
{
    ui->stackedWidget->setCurrentWidget(mSearchWidget);
    setToggledStyle(SEARCH);
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

MegaHandle NodeSelector::getSelectedNodeHandle()
{
    auto tree_view = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
    return tree_view->getSelectedNodeHandle();
}

QList<MegaHandle> NodeSelector::getMultiSelectionNodeHandle()
{
    auto tree_view = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
    return tree_view->getMultiSelectionNodeHandle();
}

void NodeSelector::closeEvent(QCloseEvent* event)
{
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
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
    ui->wLeftPane->setStyleSheet(ui->wLeftPane->styleSheet());
}

int NodeSelector::getNodeAccess(std::shared_ptr<MegaNode> node)
{
    int access = MegaShare::ACCESS_UNKNOWN;
    if (node)
    {
        access = mMegaApi->getAccess(node.get());
    }

    return access;
}

std::shared_ptr<MegaNode> NodeSelector::getSelectedNode()
{
    auto node = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
    if (!node)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again."),
                                             QMessageBox::Ok);
    }
    return node;
}


void NodeSelector::makeConnections(SelectTypeSPtr selectType)
{
    mSearchWidget = new NodeSelectorTreeViewWidgetSearch(selectType);
    mSearchWidget->setObjectName(QString::fromUtf8("Search"));
    connect(mSearchWidget, &NodeSelectorTreeViewWidgetSearch::nodeDoubleClicked, this, &NodeSelector::setSelectedNodeHandle);
    ui->stackedWidget->addWidget(mSearchWidget);
    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
        if(viewContainer)
        {
            viewContainer->init();
            connect(viewContainer, &NodeSelectorTreeViewWidget::okBtnClicked, this, &NodeSelector::onbOkClicked, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::cancelBtnClicked, this, &NodeSelector::reject, Qt::UniqueConnection);
            connect(viewContainer, &NodeSelectorTreeViewWidget::onSearch, this, &NodeSelector::onSearch, Qt::UniqueConnection);
        }
    }
}

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node, bool goToInit)
{
    if(node)
    {
        TabItem option = SHARES;
        if(mMegaApi->isInCloud(node.get()))
        {
            option = CLOUD_DRIVE;
        }
        else if(mMegaApi->isInVault(node.get()))
        {
            option = BACKUPS;
        }
        onOptionSelected(option);
        auto tree_view_widget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
        tree_view_widget->setSelectedNodeHandle(node->getHandle(), goToInit);
    }
}
