#include "StalledIssueChooseWidget.h"
#include "ui_StalledIssueChooseWidget.h"

#include "Utilities.h"
#include "MegaApplication.h"
#include "StalledIssueHeader.h"

StalledIssueChooseWidget::StalledIssueChooseWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueChooseWidget)
{
    ui->setupUi(this);
    setIndent();

    ui->chooseTitle->installEventFilter(this);
    ui->fileNameText->installEventFilter(this);

    ui->chooseTitle->addActionButton(tr("Choose"), 0);
    connect(ui->chooseTitle, &StalledIssueActionTitle::actionClicked, this, &StalledIssueChooseWidget::chooseButtonClicked);
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::setData(StalledIssueDataPtr data)
{
    mData = data;
    auto fileName = mData->getFileName();

    ui->chooseTitle->setTitle(mData->isCloud() ? tr("Remote Copy") : tr("Local Copy"));
    ui->fileNameText->setText(fileName);

    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                       fileName, QLatin1Literal(":/images/drag_")));
    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));
    ui->fileSize->setText(Utilities::getSizeString((unsigned long long)50));

    ui->path->show();
    ui->path->updateUi(data);

    if(mData->isCloud())
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(mData->getFilePath().toStdString().c_str()));
        if(node)
        {
            ui->fileSize->setText(Utilities::getSizeString((long long)node->getSize()));
        }
    }
    else
    {
        QFile file(mData->getNativeFilePath());
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

void StalledIssueChooseWidget::setIndent()
{
    ui->chooseTitle->setIndent(StalledIssueHeader::ICON_INDENT);

    auto fileNameMargins= ui->fileNameContainer->contentsMargins();
    fileNameMargins.setLeft(StalledIssueHeader::ICON_INDENT);
    ui->fileNameContainer->setContentsMargins(fileNameMargins);

    ui->path->setIndent(StalledIssueHeader::ICON_INDENT);
}

bool StalledIssueChooseWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(mData && watched == ui->fileNameText && event->type() == QEvent::Resize)
    {
        auto elidedText = ui->fileNameText->fontMetrics().elidedText(mData->getFileName(),Qt::ElideMiddle, ui->fileNameText->width());
        ui->fileNameText->setText(elidedText);
    }

    return QFrame::eventFilter(watched, event);
}
