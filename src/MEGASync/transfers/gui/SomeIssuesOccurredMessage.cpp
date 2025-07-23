#include "SomeIssuesOccurredMessage.h"

#include "DialogOpener.h"
#include "RefreshAppChangeEvent.h"
#include "StalledIssuesDialog.h"
#include "ui_SomeIssuesOccurredMessage.h"

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

bool SomeIssuesOccurredMessage::event(QEvent* event)
{
    if (RefreshAppChangeEvent::isRefreshEvent(event))
    {
        ui->retranslateUi(this);
    }

    return QWidget::event(event);
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
