#include "StalledIssueFilePath.h"
#include "ui_StalledIssueFilePath.h"

#include "Utilities.h"
#include "Platform.h"
#include "QMegaMessageBox.h"

#include <QPainter>
#include <QPoint>

const char* StalledIssueFilePath::FULL_PATH = "fullPath";

StalledIssueFilePath::StalledIssueFilePath(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueFilePath)
{
    ui->setupUi(this);
    ui->filePathAction->hide();
    ui->moveFilePathAction->hide();

    ui->moveLines->installEventFilter(this);
    ui->lines->installEventFilter(this);

    ui->filePathContainer->installEventFilter(this);
    ui->moveFilePathContainer->installEventFilter(this);

    auto openIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/ic-open-outside.png"));
    ui->filePathAction->setPixmap(openIcon.pixmap(ui->filePathAction->size()));
    ui->moveFilePathAction->setPixmap(openIcon.pixmap(ui->moveFilePathAction->size()));
}

StalledIssueFilePath::~StalledIssueFilePath()
{
    delete ui;
}

void StalledIssueFilePath::setIndent(int indent)
{
    ui->indent->changeSize(indent,0,QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->gridLayout->invalidate();
}

void StalledIssueFilePath::updateUi(QExplicitlySharedDataPointer<StalledIssueData> data)
{
    mData = data;

    fillFilePath();
    fillMoveFilePath();
}

void StalledIssueFilePath::fillFilePath()
{
    if(mData->isCloud())
    {
        auto remoteIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/cloud_upload_item_ico.png"));
        ui->LocalOrRemoteIcon->setPixmap(remoteIcon.pixmap(ui->LocalOrRemoteIcon->size()));

        ui->LocalOrRemoteText->setText(tr("on MEGA:"));
    }
    else
    {
        auto localIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/PC_ico_rest.png"));
        ui->LocalOrRemoteIcon->setPixmap(localIcon.pixmap(ui->LocalOrRemoteIcon->size()));

        ui->LocalOrRemoteText->setText(tr("Local:"));
    }

    StalledIssueData::Path data;
    data.path = mData->getNativePath();
    data.isBlocked = mData->getPath().isBlocked;
    data.isMissing = mData->getPath().isMissing;

    fillPathName(data, ui->filePath);
}

void StalledIssueFilePath::fillMoveFilePath()
{
    if(mData->hasMoveInfo())
    {
        StalledIssueData::Path data;
        data.path = mData->getNativeMovePath();
        data.isBlocked = mData->getMovePath().isBlocked;
        data.isMissing = mData->getMovePath().isMissing;

        fillPathName(data, ui->moveFilePath);
    }
    else
    {
        ui->moveFile->hide();
    }
}

void StalledIssueFilePath::fillPathName(StalledIssueData::Path data, QLabel* label)
{
    bool mInRed(false);
    if(data.isMissing)
    {
        data.path.append(QString::fromUtf8(" (missing)"));

        mInRed = true;
    }
    else
    {
        if(data.isBlocked)
        {
            data.path.append(QString::fromUtf8(" (blocked)"));

            mInRed = true;
        }
    }

    //for elided text in real time
    label->installEventFilter(this);
    label->setProperty(FULL_PATH, data.path);

    label->setText(data.path);

    if(mInRed)
    {
        label->setStyleSheet(QStringLiteral("color: red;"));
    }
    else
    {
        label->setStyleSheet(QString());
    }

    QIcon fileTypeIcon;
    if(data.isMissing)
    {
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/help-circle.png"));
    }
    else
    {
        QFileInfo fileInfo(data.path);
        if(fileInfo.isFile())
        {
            //Without extension
            if(data.path.endsWith(fileInfo.baseName()))
            {
                fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/help-circle.png"));
            }
            else
            {
                fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                              fileInfo.fileName(), QLatin1Literal(":/images/drag_")));
            }
        }
        else
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/color_folder.png"));
        }
    }

    ui->filePathIcon->setPixmap(fileTypeIcon.pixmap(ui->filePathIcon->size()));
    ui->moveFilePathIcon->setPixmap(fileTypeIcon.pixmap(ui->moveFilePathIcon->size()));
}


bool StalledIssueFilePath::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lines && event->type() == QEvent::Paint)
    {
        QPainter p(ui->lines);
        p.setPen(QPen(QColor("#000000"),1));
        p.setOpacity(0.2);

        auto width(ui->lines->width());
        auto height(ui->lines->height());

        p.drawLine(QPoint(width/2,0), QPoint(width/2, ui->lines->height()/2));
        p.drawLine(QPoint(width/2,height/2), QPoint(width, height/2));
    }
    else if(watched == ui->moveLines && event->type() == QEvent::Paint)
    {
        QPainter p(ui->moveLines);
        p.setPen(QPen(QColor("#000000"),1));
        p.setOpacity(0.2);

        auto width(ui->lines->width());
        auto height(ui->lines->height());

        p.drawLine(QPoint(0,height/2), QPoint(width, height/2));
        p.drawLine(QPoint(0,0), QPoint(0, ui->lines->height()));
    }
    else if(event->type() == QEvent::Enter || event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonRelease)
    {
        if(watched == ui->filePathContainer)
        {
            showHoverAction(event->type(), ui->filePathAction, mData->getNativeFilePath());
        }
        else if(watched == ui->moveFilePathContainer)
        {
            showHoverAction(event->type(), ui->moveFilePathAction,  mData->getNativeMoveFilePath());
        }
    }
    else if(event->type() == QEvent::Resize)
    {
        if(auto label = dynamic_cast<QLabel*>(watched))
        {
            auto fullPath = label->property(FULL_PATH);
            if(fullPath.isValid())
            {
                label->setText(label->fontMetrics().elidedText(fullPath.toString(), Qt::ElideMiddle,label->width()));
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueFilePath::showHoverAction(QEvent::Type type, QWidget *actionWidget, const QString& path)
{
    if(type == QEvent::Enter)
    {
        actionWidget->show();
        actionWidget->parent()->setProperty("itsHover", true);
        setStyleSheet(styleSheet());
    }
    else if(type == QEvent::Leave)
    {
        actionWidget->hide();
        actionWidget->parent()->setProperty("itsHover", false);
        setStyleSheet(styleSheet());
    }
    else if(type == QEvent::MouseButtonRelease)
    {
        if(mData->isCloud())
        {
            mega::MegaNode* node (MegaSyncApp->getMegaApi()->getNodeByPath(path.toStdString().c_str()));
            if (node)
            {
                const char* handle = node->getBase64Handle();
                QString url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);
                QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
                delete [] handle;
                delete node;
            }
            else
            {
                QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("Node %1 does not exist.").arg(path));
            }
        }
        else
        {
            QFile file(path);
            if(file.exists())
            {
                QtConcurrent::run([=]
                {
                    Platform::showInFolder(path);
                });
            }
            else
            {
                QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("Path %1 does not exist.").arg(path));
            }
        }
    }
}
