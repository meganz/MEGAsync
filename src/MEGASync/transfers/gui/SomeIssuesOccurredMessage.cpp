#include "SomeIssuesOccurredMessage.h"
#include "ui_SomeIssuesOccurredMessage.h"

#include <StalledIssuesDialog.h>
#include <Platform.h>

QPointer<StalledIssuesDialog> SomeIssuesOccurredMessage::mStalledIssuesDialog = nullptr;

SomeIssuesOccurredMessage::SomeIssuesOccurredMessage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SomeIssuesOccurredMessage)
{
    ui->setupUi(this);
}

SomeIssuesOccurredMessage::~SomeIssuesOccurredMessage()
{
    delete ui;
}

void SomeIssuesOccurredMessage::on_viewIssuesButton_clicked()
{
    if(!mStalledIssuesDialog)
    {
        mStalledIssuesDialog = new StalledIssuesDialog();
    }

    mStalledIssuesGeometryRetainer.showDialog(mStalledIssuesDialog);
}
