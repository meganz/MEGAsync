#include "StalledIssueChooseWidget.h"
#include "ui_StalledIssueChooseWidget.h"

#include "Utilities.h"
#include "PlatformStrings.h"
#include "MegaApplication.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesModel.h"


static const int BUTTON_ID = 0;

StalledIssueChooseWidget::StalledIssueChooseWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueChooseWidget)
{
    ui->setupUi(this);

    ui->name->removeBackgroundColor();

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::showRawInfoChanged, this, &StalledIssueChooseWidget::onRawInfoToggled);

    ui->path->setIndent(StalledIssueHeader::GROUPBOX_CONTENTS_INDENT);
    ui->path->hideLocalOrRemoteTitle();

    ui->chooseTitle->addActionButton(QIcon(), tr("Choose"), BUTTON_ID, true);
    connect(ui->chooseTitle, &StalledIssueActionTitle::actionClicked, this, &StalledIssueChooseWidget::onActionClicked);
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::updateUi(StalledIssueDataPtr data,
                                        LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
    auto fileName = data->getFileName();
    ui->chooseTitle->showIcon();

    ui->name->setTitle(fileName);

    mega::MegaHandle handle(mega::INVALID_HANDLE);
    if(data->isCloud())
    {
        auto cloudData = data->convert<CloudStalledIssueData>();
        if(cloudData)
        {
            handle = cloudData->getPathHandle();
        }
    }

    ui->name->setInfo(data->getNativeFilePath(), handle);
    ui->name->setIsFile(data->isFile());
    ui->name->setIsCloud(data->isCloud());
    ui->name->showIcon();

    ui->path->show();
    ui->path->updateUi(data);

    if((side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::None) !=
                  ui->chooseTitle->isSolved())
    {
        ui->chooseTitle->hideActionButton(BUTTON_ID);

        QIcon icon;
        if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::Remote)
        {
            if(data->isCloud())
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                ui->chooseTitle->setMessage(tr("Chosen"), icon.pixmap(24,24));
            }
            else
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                ui->chooseTitle->setMessage(solvedString(), icon.pixmap(24,24));
            }
        }
        else if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::Local)
        {
            if(data->isCloud())
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                ui->chooseTitle->setMessage(solvedString(), icon.pixmap(24,24));
            }
            else
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                ui->chooseTitle->setMessage(tr("Local file is being uploaded"), icon.pixmap(16,16));
            }
        }
        else if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::Both)
        {
            icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));

            if(data->isCloud())
            {
                auto cloudData = data->convert<CloudStalledIssueData>();
                auto node(cloudData->getNode(true));
                if(node)
                {
                    ui->chooseTitle->setMessage(tr("Renamed to %1").arg(QString::fromUtf8(node->getName())), icon.pixmap(24,24));
                }
            }
            else
            {
                ui->chooseTitle->setMessage(QString());
            }
        }
    }

    if(side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::None)
    {
        setSolved();
    }

    update();

    mData = data;
}

const StalledIssueDataPtr &StalledIssueChooseWidget::data()
{
    return mData;
}

void StalledIssueChooseWidget::hideActionButton()
{
    ui->chooseTitle->hideActionButton(BUTTON_ID);
}

bool StalledIssueChooseWidget::eventFilter(QObject *watched, QEvent *event)
{
    return QFrame::eventFilter(watched, event);
}

void StalledIssueChooseWidget::onActionClicked(int button_id)
{
    QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::AltModifier));
    qApp->processEvents();

    emit chooseButtonClicked(button_id);
}

void StalledIssueChooseWidget::setSolved()
{
    if(!ui->pathContainer->graphicsEffect())
    {
        ui->chooseTitle->setSolved(true);
        ui->name->setSolved(true);
    }
}

//LOCAL
QString LocalStalledIssueChooseWidget::solvedString() const
{
    return PlatformStrings::movedFileToBin();
}

void LocalStalledIssueChooseWidget::updateUi(LocalStalledIssueDataPtr localData,
                                             LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
   updateExtraInfo(localData);
   ui->chooseTitle->setTitle(tr("Local Copy"));
   ui->chooseTitle->setIsCloud(false);

   StalledIssueChooseWidget::updateUi(localData,side);
}

void LocalStalledIssueChooseWidget::onRawInfoToggled()
{
    updateExtraInfo(mData->convert<LocalStalledIssueData>());
}

void LocalStalledIssueChooseWidget::updateExtraInfo(LocalStalledIssueDataPtr localData)
{
    localData->getFileFolderAttributes()->requestModifiedTime(this, [this](const QDateTime& time){
        ui->name->updateLastTimeModified(time);
    });

#ifndef Q_OS_LINUX
    localData->getFileFolderAttributes()->requestCreatedTime(this, [this](const QDateTime& time){
        ui->name->updateCreatedTime(time);
    });
#endif

    localData->getFileFolderAttributes()->requestSize(this, [this](qint64 size){
        ui->name->updateSize(size);
    });

    if(MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible())
    {
        localData->getFileFolderAttributes()->requestCRC(this, [this](const QString& fp)
        {
            ui->name->updateCRC(fp);
        });
    }
    else
    {
        ui->name->updateCRC(QString());
    }
}

//CLOUD
QString CloudStalledIssueChooseWidget::solvedString() const
{
    return tr("Moved to MEGA Bin");
}

void CloudStalledIssueChooseWidget::updateUi(CloudStalledIssueDataPtr cloudData,
                                             LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
    updateExtraInfo(cloudData);
    ui->chooseTitle->setTitle(tr("Remote Copy"));
    ui->chooseTitle->setIsCloud(true);

    StalledIssueChooseWidget::updateUi(cloudData, side);
}

void CloudStalledIssueChooseWidget::onRawInfoToggled()
{
    updateExtraInfo(mData->convert<CloudStalledIssueData>());
}

void CloudStalledIssueChooseWidget::updateExtraInfo(CloudStalledIssueDataPtr cloudData)
{
    auto node = cloudData->getNode();
    if(node)
    {
        cloudData->getFileFolderAttributes()->requestModifiedTime(this, [this](const QDateTime& time){
            ui->name->updateLastTimeModified(time);
        });

        cloudData->getFileFolderAttributes()->requestCreatedTime(this, [this](const QDateTime& time){
            ui->name->updateCreatedTime(time);
        });

        cloudData->getFileFolderAttributes()->requestSize(this, [this](qint64 size){
            ui->name->updateSize(size);
        });

        if(MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible())
        {
            cloudData->getFileFolderAttributes()->requestCRC(this, [this](const QString& fp)
            {
                ui->name->updateCRC(fp);
            });
        }
        else
        {
            ui->name->updateCRC(QString());
        }

        cloudData->getFileFolderAttributes()->requestVersions(this, [this](int versions){
                ui->name->updateVersionsCount(versions);
        });

        cloudData->getFileFolderAttributes()->requestUser(this, MegaSyncApp->getMegaApi()->getMyUserHandleBinary(), [this](QString user, bool show){
            ui->name->updateUser(user, show);
        });
    }
}

//Generic options
QString GenericChooseWidget::solvedString() const
{
    return mInfo.solvedText;
}

void GenericChooseWidget::setChosen(bool state)
{
    if(state)
    {
        QIcon solvedIcon(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
        ui->chooseTitle->setMessage(mInfo.solvedText, solvedIcon.pixmap(16,16));
    }
    else
    {
        ui->chooseTitle->setMessage(QString());
    }

    hideActionButton();
}

void GenericChooseWidget::setInfo(const GenericInfo &info)
{
    mInfo = info;

    ui->pathContainer->hide();
    ui->nameContainer->hide();

    auto margins(ui->titleContainer->layout()->contentsMargins());
    margins.setTop(0);
    margins.setBottom(0);
    ui->titleContainer->layout()->setContentsMargins(margins);
    ui->chooseTitle->removeBackgroundColor();

    QIcon icon(info.icon);
    auto iconPixmap(icon.pixmap(QSize(16,16)));
    ui->chooseTitle->setHTML(info.title, iconPixmap);
    ui->chooseTitle->setActionButtonInfo(QIcon(), info.buttonText, BUTTON_ID);
}
