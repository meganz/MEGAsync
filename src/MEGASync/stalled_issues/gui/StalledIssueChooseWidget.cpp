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

void StalledIssueChooseWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor("#D6D6D6"), 1));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawRoundedRect(QRectF(0.0,0.0,width(), height()),6,6);
}

bool StalledIssueChooseWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->chooseTitle && event->type() == QEvent::Paint)
    {
        QPainter painter(ui->chooseTitle);
        painter.setBrush(QColor("#F5F5F5"));
        painter.setPen(Qt::NoPen);
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRoundedRect( QRect(0,0, ui->chooseTitle->width(), 6), 6, 6);
        path.addRect(QRect( 0, 3, ui->chooseTitle->width(), ui->chooseTitle->height() -3)); // Top right corner not rounded
        painter.drawPath(path.simplified());
    }
    else if(mData && watched == ui->fileNameText && event->type() == QEvent::Resize)
    {
        auto elidedText = ui->fileNameText->fontMetrics().elidedText(mData->getFileName(),Qt::ElideMiddle, ui->fileNameText->width());
        ui->fileNameText->setText(elidedText);
    }

    return QFrame::eventFilter(watched, event);
}
