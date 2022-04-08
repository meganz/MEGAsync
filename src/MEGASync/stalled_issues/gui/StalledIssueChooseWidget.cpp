#include "StalledIssueChooseWidget.h"
#include "ui_StalledIssueChooseWidget.h"

#include "Utilities.h"
#include "MegaApplication.h"

StalledIssueChooseWidget::StalledIssueChooseWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueChooseWidget)
{
    ui->setupUi(this);

    ui->CloudPath->hide();
    ui->LocalPath->hide();

    connect(ui->chooseButton, &QPushButton::clicked, this, &StalledIssueChooseWidget::chooseButtonClicked);
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::setData(StalledIssueDataPtr data, const QString& fileName)
{
    mData = data;

    ui->titleLabel->setText(mData->mIsCloud ? tr("Remote Copy") : tr("Local Copy"));
    ui->fileNameText->setText(fileName);

    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                       fileName, QLatin1Literal(":/images/drag_")));
    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));
    ui->fileSize->setText(Utilities::getSizeString((unsigned long long)50));

    ui->LocalPath->show();

    StalledIssue stalledData(data);
    ui->LocalPath->updateUi(QModelIndex(), stalledData);

    if(mData->mIsCloud)
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(mData->mIndexPath.toStdString().c_str()));
        if(node)
        {
            ui->fileSize->setText(Utilities::getSizeString(node->getSize()));
        }
    }
    else
    {
        QFile file(mData->mIndexPath);
        if(file.exists())
        {
            ui->fileSize->setText(Utilities::getSizeString(file.size()));
        }
    }
}

const StalledIssueDataPtr &StalledIssueChooseWidget::data()
{
    return mData;
}
