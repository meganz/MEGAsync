#include "MegaInfoMessage.h"
#include "ui_MegaInfoMessage.h"

MegaInfoMessage::MegaInfoMessage(const QString &windowTitle, const QString &title, const QString &firstParagraph,
                                 const QString &secondParagraph, const QIcon &icon, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MegaInfoMessage)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->setWindowTitle(windowTitle);
    ui->lInfoTitle->setText(title);
    ui->lFirstDescP->setText(firstParagraph);

    if (!secondParagraph.isEmpty())
    {
        ui->lSecondDescP->setText(secondParagraph);
    }

    if (!icon.isNull())
    {
        ui->bIcon->setIcon(icon);
    }
    else
    {
        ui->wIcon->hide();
    }
}

void MegaInfoMessage::on_bClose_clicked()
{
    accept();
}


MegaInfoMessage::~MegaInfoMessage()
{
    delete ui;
}
