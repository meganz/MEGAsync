#include "StalledIssueChooseWidget.h"
#include "ui_StalledIssueChooseWidget.h"

#include "Utilities.h"
#include "MegaApplication.h"

StalledIssueChooseWidget::StalledIssueChooseWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueChooseWidget)
{
    ui->setupUi(this);

    ui->CloudPath->hide();
    ui->LocalPath->hide();
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::refreshUi()
{
    ui->titleLabel->setText(getData()->mIsCloud ? tr("Remote Copy") : tr("Local Copy"));
    ui->fileNameText->setText(getData()->mFileName);

    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                       getData()->mFileName, QLatin1Literal(":/images/drag_")));
    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));
    ui->fileSize->setText(Utilities::getSizeString((unsigned long long)50));

    ui->LocalPath->show();
    ui->LocalPath->updateUi(getCurrentIndex(), getData());

    if(getData()->mIsCloud)
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(getData()->mIndexPath.toStdString().c_str()));
        if(node)
        {
            ui->fileSize->setText(Utilities::getSizeString(node->getSize()));
        }
    }
    else
    {
        QFile file(getData()->mIndexPath);
        if(file.exists())
        {
            ui->fileSize->setText(Utilities::getSizeString(file.size()));
        }
    }
}

void StalledIssueChooseWidget::paintEvent(QPaintEvent *event)
{

}
