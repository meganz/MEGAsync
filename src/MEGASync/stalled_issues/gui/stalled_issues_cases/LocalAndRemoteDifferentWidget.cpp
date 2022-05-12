#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"

#include "mega/types.h"

#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent), mega::MegaRequestListener(),
    ui(new Ui::LocalAndRemoteDifferentWidget),
    mListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mRemovedRemoteHandle(0)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::ICON_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    auto issue = getData();

    if(issue.getLocalData())
    {
        ui->chooseLocalCopy->setData(issue.getLocalData());
    }

    if(issue.getCloudData())
    {
        ui->chooseRemoteCopy->setData(issue.getCloudData());
    }
}

void LocalAndRemoteDifferentWidget::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_MOVE)
    {
        if (e->getErrorCode() == mega::MegaError::API_OK)
        {
            auto handle = request->getNodeHandle();
            if(handle && handle == mRemovedRemoteHandle)
            {
                emit issueFixed();
                mRemovedRemoteHandle = 0;
            }
        }
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked()
{ 
    auto fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toStdString().c_str()));
    if(fileNode)
    {
        mRemovedRemoteHandle = fileNode->getHandle();
        auto rubbishNode = MegaSyncApp->getMegaApi()->getRubbishNode();
        MegaSyncApp->getMegaApi()->moveNode(fileNode,rubbishNode, mListener.get());
    }
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked()
{
    QFile file(ui->chooseLocalCopy->data()->getNativeFilePath());
    if(file.exists())
    {
         if(Utilities::moveFileToTrash(ui->chooseLocalCopy->data()->getNativeFilePath()))
         {
             emit issueFixed();
         }
    }
}
