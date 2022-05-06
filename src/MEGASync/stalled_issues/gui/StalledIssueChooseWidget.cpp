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

    connect(ui->chooseButton, &QPushButton::clicked, this, &StalledIssueChooseWidget::chooseButtonClicked);
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::setData(StalledIssueDataPtr data, const QString& fileName)
{
    mData = data;
    mFileName = fileName;

    ui->titleLabel->setText(mData->mIsCloud ? tr("Remote Copy") : tr("Local Copy"));
    ui->fileNameText->setText(fileName);

    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                       fileName, QLatin1Literal(":/images/drag_")));
    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));
    ui->fileSize->setText(Utilities::getSizeString((unsigned long long)50));

    ui->path->show();

    StalledIssue stalledData(data);
    ui->path->updateUi(QModelIndex(), stalledData);

    if(mData->mIsCloud)
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(mData->mPath.path.toStdString().c_str()));
        if(node)
        {
            ui->fileSize->setText(Utilities::getSizeString((long long)node->getSize()));
        }
    }
    else
    {
        QFile file(mData->mPath.path);
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
    auto chooseMargins = ui->chooseTitle->contentsMargins();
    chooseMargins.setLeft(StalledIssueHeader::ICON_INDENT);
    ui->chooseTitle->setContentsMargins(chooseMargins);

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
    else if(watched == ui->fileNameText && event->type() == QEvent::Resize)
    {
        auto elidedText = ui->fileNameText->fontMetrics().elidedText(mFileName,Qt::ElideMiddle, ui->fileNameText->width());
        ui->fileNameText->setText(elidedText);
    }

    return QFrame::eventFilter(watched, event);
}
