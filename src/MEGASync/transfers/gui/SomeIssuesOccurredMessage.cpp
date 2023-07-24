#include "SomeIssuesOccurredMessage.h"
#include "ui_SomeIssuesOccurredMessage.h"

#include <StalledIssuesDialog.h>
#include <DialogOpener.h>
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
    auto stalledIssuesDialog = DialogOpener::findDialog<StalledIssuesDialog>();
    if(stalledIssuesDialog)
    {
        DialogOpener::showGeometryRetainerDialog(stalledIssuesDialog->getDialog());
    }
    else
    {
        auto newStalledIssuesDialog = new StalledIssuesDialog();
        DialogOpener::showDialog<StalledIssuesDialog>(newStalledIssuesDialog);
    }
}
