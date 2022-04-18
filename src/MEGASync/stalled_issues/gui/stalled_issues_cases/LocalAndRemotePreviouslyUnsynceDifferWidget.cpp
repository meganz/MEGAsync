#include "LocalAndRemotePreviouslyUnsynceDifferWidget.h"
#include "ui_LocalAndRemotePreviouslyUnsynceDifferWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "mega/types.h"

#include <QFile>

LocalAndRemotePreviouslyUnsynceDifferWidget::LocalAndRemotePreviouslyUnsynceDifferWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent), mega::MegaRequestListener(),
    ui(new Ui::LocalAndRemotePreviouslyUnsynceDifferWidget),
    mListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mRemovedRemoteHandle(0)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemotePreviouslyUnsynceDifferWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemotePreviouslyUnsynceDifferWidget::onRemoteButtonClicked);
}

LocalAndRemotePreviouslyUnsynceDifferWidget::~LocalAndRemotePreviouslyUnsynceDifferWidget()
{
    delete ui;
}

void LocalAndRemotePreviouslyUnsynceDifferWidget::refreshUi()
{
    auto issue = getData();
    for(int index = 0; index < issue.stalledIssuesCount(); ++index)
    {
        auto data = issue.getStalledIssueData(index);
        data->mIsCloud ? ui->chooseRemoteCopy->setData(data, issue.getFileName()) : ui->chooseLocalCopy->setData(data, issue.getFileName());
    }
}

void LocalAndRemotePreviouslyUnsynceDifferWidget::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_REMOVE)
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

void LocalAndRemotePreviouslyUnsynceDifferWidget::onLocalButtonClicked()
{ 
    auto fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->mIndexPath.toStdString().c_str()));
    if(fileNode)
    {
        mRemovedRemoteHandle = fileNode->getHandle();
        MegaSyncApp->getMegaApi()->remove(fileNode, mListener.get());
    }
}

void LocalAndRemotePreviouslyUnsynceDifferWidget::onRemoteButtonClicked()
{
    QFile file(ui->chooseLocalCopy->data()->mIndexPath);
    if(file.exists())
    {
        if(file.remove())
        {
            emit issueFixed();
        }
    }
}
