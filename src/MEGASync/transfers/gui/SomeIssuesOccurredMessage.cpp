#include "SomeIssuesOccurredMessage.h"
#include "ui_SomeIssuesOccurredMessage.h"

#include <StalledIssuesDialog.h>
#include <Platform.h>

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

    mStalledIssuesDialog->showNormal();
    mStalledIssuesDialog->activateWindow();
    mStalledIssuesDialog->raise();
}
