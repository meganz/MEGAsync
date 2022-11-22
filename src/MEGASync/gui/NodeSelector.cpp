#include "NodeSelector.h"
#include "ui_NodeSelector.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"
#include "megaapi.h"
#include "mega/utils.h"
#include "MegaItemProxyModel.h"
#include "MegaItemModel.h"
#include <NodeSelectorTreeViewWidget.h>

#include <QMessageBox>
#include <QPointer>
#include <QShortcut>

using namespace mega;

const int NodeSelector::LABEL_ELIDE_MARGIN = 100;

const char* NodeSelector::IN_SHARES = "Incoming shares";
const char* NodeSelector::CLD_DRIVE = "Cloud drive";
const char* NodeSelector::BACKUPS = "Backups";


NodeSelector::NodeSelector(int selectMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeSelector),
    mSelectMode(selectMode),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowModality(Qt::WindowModal);
    ui->setupUi(this);

#ifndef Q_OS_MAC
    connect(ui->bShowIncomingShares, &QPushButton::clicked, this, &NodeSelector::onbShowIncomingSharesClicked);
    connect(ui->bShowCloudDrive, &QPushButton::clicked, this, &NodeSelector::onbShowCloudDriveClicked);
    connect(ui->bShowBackups, &QPushButton::clicked, this, &NodeSelector::onbShowBackupsFolderClicked);
#else
    ui->tabBar->addTab(tr(CLD_DRIVE));
    ui->tabBar->addTab(tr(IN_SHARES));
    ui->tabBar->addTab(tr(BACKUPS));

    connect(ui->tabBar, &QTabBar::currentChanged, this, &NodeSelector::onOptionSelected);
#endif

    for(int page = 0; page < ui->stackedWidget->count(); ++page)
    {
        auto viewContainer = dynamic_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->widget(page));
        if(viewContainer)
        {
            if(page == VAULT && (mSelectMode == NodeSelector::SYNC_SELECT || mSelectMode == NodeSelector::UPLOAD_SELECT))
            {
                ui->stackedWidget->removeWidget(viewContainer);
                hideSelector((NodeSelector::TabItem)page);
                viewContainer->deleteLater();
            }
            else
            {
                viewContainer->setSelectionMode(mSelectMode);
                connect(viewContainer, &NodeSelectorTreeViewWidget::okBtnClicked, this, &NodeSelector::onbOkClicked);
                connect(viewContainer, &NodeSelectorTreeViewWidget::cancelBtnClicked, this, &NodeSelector::reject);
                connect(viewContainer, &NodeSelectorTreeViewWidget::onViewReady, this, &NodeSelector::onViewReady);
            }

        }
    }

    //TODO EKA: WE need to do this at this lvl? only for stream_select mode, switch removed
    //setWindowTitle(tr("Select items"));
    if(mSelectMode == STREAM_SELECT)
    {
        setWindowTitle(tr("Select items"));
    }
}

NodeSelector::~NodeSelector()
{
    delete ui;
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
            viewContainer->showDefaultUploadOption(value);
        }
    }
}

bool NodeSelector::getDefaultUploadOption()
{
    return ui->CloudDrive->getDefaultUploadOption();
}

void NodeSelector::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void NodeSelector::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
}

void NodeSelector::onbOkClicked()
{
    bool correctNodeSelected(true);

    if(mSelectMode == NodeSelector::DOWNLOAD_SELECT)
    {
        auto treeViewWidget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
        QList<MegaHandle> nodes = treeViewWidget->getMultiSelectionNodeHandle();
        int wrongNodes(0);
        foreach(auto& nodeHandle, nodes)
        {
            auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(nodeHandle));
            if(!node)
            {
                ++wrongNodes;
            }
        }

        if(wrongNodes == nodes.size())
        {
            correctNodeSelected = false;
            if(ui->stackedWidget->currentIndex() == CLOUD_DRIVE)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again.", "", wrongNodes), QMessageBox::Ok);
            }
            else
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You no longer have access to this item. Ask the owner to share again.", "", wrongNodes), QMessageBox::Ok);
            }
        }
        else if(wrongNodes > 0)
        {
            correctNodeSelected = false;
            QString warningMsg1 = tr("%1 item selected", "", nodes.size()).arg(nodes.size());
            QString warningMsg = tr("%1. %2 has been removed. To reselect, close this window and try again.", "", wrongNodes).arg(warningMsg1).arg(wrongNodes);
            QMegaMessageBox::warning(nullptr, tr("Error"), warningMsg, QMessageBox::Ok);
        }
    }
    else
    {
        auto treeViewWidget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
        auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(treeViewWidget->getSelectedNodeHandle()));
        if (!node)
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("The item you selected has been removed. To reselect, close this window and try again."),
                                                 QMessageBox::Ok);
            correctNodeSelected = false;
        }
        else
        {
            int access = mMegaApi->getAccess(node.get());
            if ((mSelectMode == NodeSelector::UPLOAD_SELECT) && ((access < MegaShare::ACCESS_READWRITE)))
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Read & Write or Full access rights to be able to upload to the selected folder."), QMessageBox::Ok);
                correctNodeSelected = false;
            }
            else if ((mSelectMode == NodeSelector::SYNC_SELECT) && (access < MegaShare::ACCESS_FULL))
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("You need Full access right to be able to sync the selected folder."), QMessageBox::Ok);
                correctNodeSelected = false;
            }
            else if ((mSelectMode == NodeSelector::STREAM_SELECT) && node->isFolder())
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("Only files can be used for streaming."), QMessageBox::Ok);
                correctNodeSelected = false;
            }
            else if (mSelectMode == NodeSelector::SYNC_SELECT)
            {
                const char* path = mMegaApi->getNodePath(node.get());
                auto check = std::unique_ptr<MegaNode>(mMegaApi->getNodeByPath(path));
                delete [] path;
                if (!check)
                {
                    QMegaMessageBox::warning(nullptr, tr("Warning"), tr("Invalid folder for synchronization.\n"
                                                         "Please, ensure that you don't use characters like '\\' '/' or ':' in your folder names."),
                                             QMessageBox::Ok);
                    correctNodeSelected = false;
                }
            }
        }
    }
    correctNodeSelected ? accept() : reject();
}

void NodeSelector::onOptionSelected(int index)
{
#ifdef Q_OS_MAC
    onTabSelected(index);
#else
    switch (index)
    {
        case NodeSelector::CLOUD_DRIVE:
            ui->bShowCloudDrive->click();
            break;
        case NodeSelector::SHARES:
            ui->bShowIncomingShares->click();
            break;
        case NodeSelector::VAULT:
            ui->bShowBackups->click();
            break;
        default:
            break;
    }
#endif

}

#ifdef Q_OS_MAC
void NodeSelector::onTabSelected(int index)
{
    auto tabText = ui->tabBar->tabText(index);
    if(tabText == tr(CLD_DRIVE))
    {
        onbShowCloudDriveClicked();
    }
    else if(tabText == tr(IN_SHARES))
    {
        onbShowIncomingSharesClicked();
    }
    else if(tabText == tr(BACKUPS))
    {
        onbShowBackupsFolderClicked();
    }

    ui->tabBar->setCurrentIndex(index);
}
#endif

void NodeSelector::onbShowCloudDriveClicked()
{
    ui->stackedWidget->setCurrentIndex(CLOUD_DRIVE);
}

void NodeSelector::onbShowIncomingSharesClicked()
{
    ui->stackedWidget->setCurrentIndex(SHARES);
}

void NodeSelector::onbShowBackupsFolderClicked()
{
    qDebug() << ui->stackedWidget->currentIndex() << VAULT;
    ui->stackedWidget->setCurrentIndex(VAULT);
    qDebug() << ui->stackedWidget->currentIndex();
}

void NodeSelector::onViewReady(bool isEmpty)
{
    if(sender() == ui->Backups && isEmpty)
    {
        hideSelector(VAULT);
        shortCutConnects(VAULT);
    }
    else if(sender() == ui->IncomingShares && isEmpty)
    {
        hideSelector(SHARES);
        shortCutConnects(SHARES);
    }
}

void NodeSelector::shortCutConnects(int ignoreThis)
{
    // Provide quick access shortcuts for the two panes via Ctrl+1,2
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i <= VAULT; ++i)
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
            processCloseEvent(viewContainer->getProxyModel(), event);
        }
    }
}

void NodeSelector::processCloseEvent(MegaItemProxyModel *proxy, QCloseEvent *event)
{
    if(proxy->isModelProcessing())
    {
        connect(proxy->getMegaModel(), &MegaItemModel::blockUi, this, [this](bool blocked){
            if(!blocked)
            {
                close();
            }
        });
        event->ignore();
    }
}

void NodeSelector::hideSelector(TabItem item)
{
    switch(item)
    {
        case CLOUD_DRIVE:
        {
#ifndef Q_OS_MAC
            ui->bShowCloudDrive->hide();
#else
            hideTabSelector(tr(CLD_DRIVE));
#endif
            break;
        }
        case SHARES:
        {
#ifndef Q_OS_MAC
            ui->bShowIncomingShares->hide();
#else
            hideTabSelector(tr(IN_SHARES));
#endif
            break;
        }
        case VAULT:
        {
#ifndef Q_OS_MAC
            ui->bShowBackups->hide();
#else
            hideTabSelector(tr(BACKUPS));
#endif
            break;
        }
    }
}

#ifdef Q_OS_MAC
void NodeSelector::hideTabSelector(const QString& tabText)
{
    for(int index = 0; index < ui->tabBar->count(); ++index)
    {
        if(ui->tabBar->tabText(index) == tabText)
        {
            ui->tabBar->removeTab(index);
            break;
        }
    }
}
#endif

void NodeSelector::setSelectedNodeHandle(std::shared_ptr<MegaNode> node)
{
    if(!node)
    {
        node = std::shared_ptr<MegaNode>(mMegaApi->getRootNode());
    }

    if(node)
    {
        mega::MegaHandle originHandle = node->getHandle();
        while(node->getParentHandle() != INVALID_HANDLE)
        {
            node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(node->getParentHandle()));
        }

        if(node->isInShare())
        {
            onOptionSelected(SHARES);
        }
        else
        {
            onOptionSelected(CLOUD_DRIVE);
        }

        auto tree_view_widget = static_cast<NodeSelectorTreeViewWidget*>(ui->stackedWidget->currentWidget());
        tree_view_widget->setSelectedNodeHandle(originHandle);
    }
}
