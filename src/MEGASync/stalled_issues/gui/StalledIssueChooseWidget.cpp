#include "StalledIssueChooseWidget.h"
#include "ui_StalledIssueChooseWidget.h"

#include "Utilities.h"
#include "MegaApplication.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesModel.h"

static const int BUTTON_ID = 0;

StalledIssueChooseWidget::StalledIssueChooseWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueChooseWidget),
    mIsSolved(false),
    mPreviousSolveState(false)
{
    ui->setupUi(this);

    ui->path->setIndent(StalledIssueHeader::GROUPBOX_CONTENTS_INDENT);
    auto layoutMargins = ui->fileNameContainer->contentsMargins();
    layoutMargins.setLeft(StalledIssueHeader::StalledIssueHeader::GROUPBOX_CONTENTS_INDENT);
    ui->fileNameContainer->setContentsMargins(layoutMargins);

    ui->chooseTitle->installEventFilter(this);
    ui->fileNameText->installEventFilter(this);

    ui->chooseTitle->addActionButton(QIcon(), tr("Choose"), BUTTON_ID);
    connect(ui->chooseTitle, &StalledIssueActionTitle::actionClicked, this, &StalledIssueChooseWidget::onActionClicked);
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::setData(StalledIssueDataPtr data)
{
    auto fileName = data->getFileName();

    ui->chooseTitle->setTitle(data->isCloud() ? tr("Remote Copy") : tr("Local Copy"));
    ui->fileNameText->setText(fileName);

    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                       fileName, QLatin1Literal(":/images/drag_")));
    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));
    ui->fileSize->setText(Utilities::getSizeString((unsigned long long)50));

    ui->path->show();
    ui->path->updateUi(data);

    if(data->isCloud())
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(data->getFilePath().toStdString().c_str()));
        if(node)
        {
            ui->fileSize->setText(Utilities::getSizeString((long long)node->getSize()));
        }
    }
    else
    {
        QFile file(data->getNativeFilePath());
        if(file.exists())
        {
            ui->fileSize->setText(Utilities::getSizeString(file.size()));
        }
    }

    if(mPreviousSolveState != mIsSolved)
    {
        mPreviousSolveState = mIsSolved;

        if(mIsSolved)
        {
            ui->chooseTitle->setDisabled(true);
            ui->chooseTitle->hideActionButton(BUTTON_ID);
            if(data->isSolved())
            {
                ui->chooseTitle->addMessage(tr("Choosen"));
            }
        }
    }

    mData = data;
}

const StalledIssueDataPtr &StalledIssueChooseWidget::data()
{
    return mData;
}

void StalledIssueChooseWidget::setIndent()
{

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

void StalledIssueChooseWidget::onActionClicked(int button_id)
{
    emit chooseButtonClicked(button_id);
}

void StalledIssueChooseWidget::setIssueSolved(bool newIssueSolved)
{
    mIsSolved = newIssueSolved;
}
