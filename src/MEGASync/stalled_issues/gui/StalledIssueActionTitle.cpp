#include "StalledIssueActionTitle.h"
#include "ui_StalledIssueActionTitle.h"

#include <Utilities.h>
#include <MegaApplication.h>
#include <StalledIssuesUtilities.h>
#include <StalledIssuesModel.h>

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QSpacerItem>

const char* BUTTON_ID = "button_id";
const char* ONLY_ICON = "onlyIcon";
const char* MAIN_BUTTON = "main";
const char* DISCARDED = "discarded";
const char* DISABLE_BACKGROUND = "disable_background";
const char* MESSAGE_TEXT = "message_text";
const char* EXTRAINFO_INFO = "extrainfo_info";
const char* EXTRAINFO_SIZE = "extrainfo_size";

#include <QGraphicsOpacityEffect>

StalledIssueActionTitle::StalledIssueActionTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueActionTitle),
    mIsCloud(false)
{
    ui->setupUi(this);

    ui->icon->hide();
    ui->messageContainer->hide();
    ui->actionContainer->hide();
    ui->extraInfoContainer->hide();
    ui->generalContainer->installEventFilter(this);
    ui->titleLabel->installEventFilter(this);

    ui->backgroundWidget->setProperty(DISABLE_BACKGROUND, false);
    setSolved(false);
}

StalledIssueActionTitle::~StalledIssueActionTitle()
{
    delete ui;
}

void StalledIssueActionTitle::removeBackgroundColor()
{
    ui->backgroundWidget->setProperty(DISABLE_BACKGROUND, true);
}

void StalledIssueActionTitle::setTitle(const QString &title)
{
    layout()->activate();

    ui->backgroundWidget->layout()->activate();
    ui->actionContainer->updateGeometry();

    ui->generalContainer->layout()->activate();
    ui->generalContainer->updateGeometry();

    ui->contents->layout()->activate();
    ui->contents->updateGeometry();

    ui->titleContainer->layout()->activate();
    ui->titleContainer->updateGeometry();

    ui->titleLabel->setText(title);
    ui->titleLabel->setProperty(MESSAGE_TEXT, title);
}

QString StalledIssueActionTitle::title() const
{
    return ui->titleLabel->property(MESSAGE_TEXT).toString();
}

void StalledIssueActionTitle::addActionButton(const QIcon& icon,const QString &text, int id, bool mainButton)
{
    auto button = new QPushButton(icon, text, this);

    button->setProperty(BUTTON_ID, id);
    button->setProperty(ONLY_ICON, false);
    button->setProperty(MAIN_BUTTON,mainButton);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked, this, [this]()
    {
        QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::NoModifier));
        qApp->processEvents();
        emit actionClicked(sender()->property(BUTTON_ID).toInt());
    });

    ui->actionLayout->addWidget(button);

    if(!icon.isNull())
    {
        button->setIconSize(QSize(24,24));

        if(text.isEmpty())
        {
            button->setProperty(ONLY_ICON, true);
        }
    }

    ui->actionContainer->setStyleSheet(ui->actionContainer->styleSheet());
    ui->actionContainer->show();
}

void StalledIssueActionTitle::hideActionButton(int id)
{
    bool allHidden(true);

    auto buttons = ui->actionContainer->findChildren<QPushButton*>();
    foreach(auto& button, buttons)
    {
        if(button->property(BUTTON_ID).toInt() == id)
        {
            button->hide();
        }
        else if(button->isVisible())
        {
            allHidden = false;
        }
    }

    if(allHidden)
    {
        ui->actionContainer->hide();
    }
}

void StalledIssueActionTitle::showIcon()
{
    QIcon fileTypeIcon;
    QFileInfo fileInfo(mPath);

    if(mIsCloud)
    {
        fileTypeIcon = StalledIssuesUtilities::getRemoteFileIcon(mNode.get(), fileInfo, false);
    }
    else
    {
        fileTypeIcon = StalledIssuesUtilities::getLocalFileIcon(fileInfo, false);
    }

    ui->icon->setPixmap(fileTypeIcon.pixmap(ui->icon->size()));
    ui->icon->show();
}

void StalledIssueActionTitle::addMessage(const QString &message, const QPixmap& pixmap)
{
    QWidget* labelContainer = new QWidget(this);
    labelContainer->installEventFilter(this);
    labelContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QHBoxLayout* labelContainerLayout = new QHBoxLayout();
    labelContainerLayout->setObjectName(QString::fromLatin1("messageLayout"));
    labelContainerLayout->setContentsMargins(0,0,10,0);
    labelContainer->setLayout(labelContainerLayout);
    labelContainerLayout->addStretch();

    if(!pixmap.isNull())
    {
        auto iconLabel = new QLabel(this);
        iconLabel->setPixmap(pixmap);
        labelContainerLayout->addWidget(iconLabel);
    }

    auto messageLabel = new QLabel(message, this);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    messageLabel->setProperty(MESSAGE_TEXT, message);
    labelContainerLayout->addWidget(messageLabel);

    ui->messageContainer->show();
    ui->messageLayout->addWidget(labelContainer);
}

QLabel* StalledIssueActionTitle::addExtraInfo(const QString &title, const QString &info, int level)
{
    ui->extraInfoContainer->show();

    auto titleLabel = new QLabel(title, this);
    auto infoLabel = new QLabel(info, this);
    infoLabel->setProperty(MESSAGE_TEXT, info);
    infoLabel->setProperty(EXTRAINFO_INFO, true);

    QHBoxLayout* rowLayout(nullptr);
    auto rowItem = ui->extraInfoLayout->itemAt(level);
    if(!rowItem)
    {
        auto rowWidget = new QWidget();
        rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0,0,0,0);
        rowLayout->setSpacing(3);
        rowLayout->addStretch();

        ui->extraInfoLayout->addWidget(rowWidget);
    }
    else
    {
        rowLayout = dynamic_cast<QHBoxLayout*>(rowItem->widget()->layout());
    }

    rowLayout->insertWidget(rowLayout->count()-1,titleLabel, 0 , Qt::AlignLeft);
    rowLayout->insertWidget(rowLayout->count()-1,infoLabel, 0 , Qt::AlignLeft);
    rowLayout->insertItem(rowLayout->count()-1, new QSpacerItem(20,10,QSizePolicy::Fixed, QSizePolicy::Fixed));

    setStyleSheet(styleSheet());

    return infoLabel;
}

void StalledIssueActionTitle::setSolved(bool state)
{
    ui->backgroundWidget->setProperty(DISCARDED,state);
    setStyleSheet(styleSheet());

    if(state && !ui->titleContainer->graphicsEffect())
    {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.30);
        ui->titleContainer->setGraphicsEffect(effect);
    }

    if(state && !ui->extraInfoContainer->graphicsEffect())
    {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.30);
        ui->extraInfoContainer->setGraphicsEffect(effect);
    }
}

bool StalledIssueActionTitle::isSolved() const
{
    return ui->backgroundWidget->property(DISCARDED).toBool();
}

void StalledIssueActionTitle::setIsCloud(bool state)
{
    mIsCloud = state;
}

bool StalledIssueActionTitle::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        if(watched == ui->generalContainer)
        {
            updateExtraInfoLayout();
        }
        else
        {
            auto contentWidget = dynamic_cast<QWidget*>(watched);
            if(contentWidget)
            {
                auto width = contentWidget->width();

                if(width > (ui->contents->width()/3))
                {
                    width = ui->contents->width()/3;
                }

                auto childLabels = contentWidget->findChildren<QLabel*>();
                foreach(auto label, childLabels)
                {
                    auto elidedText = label->fontMetrics().elidedText(label->property(MESSAGE_TEXT).toString(), Qt::ElideMiddle, width);
                    label->setText(elidedText);
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueActionTitle::updateUser(const QString &user, bool show)
{
    if(show)
    {
        auto userText = user.isEmpty() ? tr("Loading user…") : user;
        auto& userLabel = mUpdateLabels[AttributeType::User];
        if(!userLabel)
        {
            userLabel = addExtraInfo(tr("Upload by:"), userText, 1);
        }
        else
        {
            updateLabel(userLabel, userText);
        }

        showAttribute(AttributeType::User);
    }
    else
    {
        hideAttribute(AttributeType::User);
    }
}

void StalledIssueActionTitle::updateVersionsCount(int versions)
{
    if(versions > 1)
    {
        QString versionsText(QString::number(versions));
        auto& versionsLabel = mUpdateLabels[AttributeType::Versions];
        if(!versionsLabel)
        {
            versionsLabel = addExtraInfo(tr("Versions:"), versionsText, 1);
        }
        else
        {
            updateLabel(versionsLabel, versionsText);
        }

        showAttribute(AttributeType::Versions);
    }
    else
    {
        hideAttribute(AttributeType::Versions);
    }
}

void StalledIssueActionTitle::updateSize(int64_t size)
{
    auto rawValues = isRawInfoVisible();
    QString sizeText;
    if(size >= 0)
    {
        sizeText = rawValues ? QString::number(size) : Utilities::getSizeString(static_cast<long long>(size));
    }
    else
    {
        sizeText = tr("Loading size");
    }

    auto& sizeLabel = mUpdateLabels[AttributeType::Size];
    if(!sizeLabel)
    {
        sizeLabel = addExtraInfo(tr("Size:"), sizeText, 0);
    }
    else
    {
        updateLabel(sizeLabel, sizeText);
    }

    showAttribute(AttributeType::Size);
}

void StalledIssueActionTitle::updateFingerprint(const QString& fp)
{
    auto rawValues = isRawInfoVisible();

    if(rawValues)
    {
        QString fpText = fp.isEmpty() ? tr("-") : fp;

        auto& fpLabel = mUpdateLabels[AttributeType::Fingerprint];
        if(!fpLabel)
        {
            fpLabel = addExtraInfo(tr("Fingerprint:"), fpText, 0);
        }
        else
        {
            updateLabel(fpLabel, fpText);
        }

        showAttribute(AttributeType::Fingerprint);
    }
    else
    {
        hideAttribute(AttributeType::Fingerprint);
    }
}

void StalledIssueActionTitle::updateLastTimeModified(const QDateTime& time)
{
    auto rawValues = isRawInfoVisible();
    QString timeString;
    if(time.isValid())
    {
        timeString = rawValues ? QString::number(time.toSecsSinceEpoch()) : MegaSyncApp->getFormattedDateByCurrentLanguage(time, QLocale::FormatType::ShortFormat);
    }
    else
    {
        timeString = tr("Loading time…");
    }

    auto& lastTimeLabel = mUpdateLabels[AttributeType::LastModified];
    if(!lastTimeLabel)
    {
        lastTimeLabel = addExtraInfo(tr("Last modified:"), timeString, 0);
    }
    else
    {
        updateLabel(lastTimeLabel, timeString);
    }

    showAttribute(AttributeType::LastModified);
}

void StalledIssueActionTitle::updateCreatedTime(const QDateTime& time)
{
    auto rawValues = isRawInfoVisible();
    QString timeString;
    if(time.isValid())
    {
        timeString = rawValues ? QString::number(time.toSecsSinceEpoch()) : MegaSyncApp->getFormattedDateByCurrentLanguage(time, QLocale::FormatType::ShortFormat);
    }
    else
    {
        timeString = tr("Loading time…");
    }

    auto& createdTimeLabel = mUpdateLabels[AttributeType::CreatedTime];
    if(!createdTimeLabel)
    {
        createdTimeLabel = addExtraInfo(mIsCloud ? tr("Upload at:") : tr("Created at:"),  timeString, 0);
    }
    else
    {
        updateLabel(createdTimeLabel, timeString);
    }

    showAttribute(AttributeType::CreatedTime);
}

void StalledIssueActionTitle::hideAttribute(AttributeType type)
{
    auto updateLabel = mUpdateLabels.value(type);
    if(updateLabel)
    {
        auto layout = dynamic_cast<QHBoxLayout*>(updateLabel->parentWidget()->layout());
        if(layout)
        {
            auto labelIndex = layout->indexOf(updateLabel);
            layout->itemAt(labelIndex)->widget()->hide();
            layout->itemAt(labelIndex-1)->widget()->hide();

            if(layout->count() == 2)
            {
                updateLabel->parentWidget()->hide();
            }
        }
    }
}

void StalledIssueActionTitle::showAttribute(AttributeType type)
{
    auto updateLabel = mUpdateLabels.value(type);
    if(updateLabel)
    {
        auto layout = dynamic_cast<QHBoxLayout*>(updateLabel->parentWidget()->layout());
        if(layout)
        {
            if(layout->count() == 2)
            {
                updateLabel->parentWidget()->show();
            }

            auto labelIndex = layout->indexOf(updateLabel);
            layout->itemAt(labelIndex)->widget()->show();
            layout->itemAt(labelIndex-1)->widget()->show();
        }
    }
}

void StalledIssueActionTitle::updateLabel(QLabel *label, const QString &text)
{
    if(label->property(EXTRAINFO_SIZE).isValid())
    {
        auto elidedText = label->fontMetrics().elidedText(text, Qt::ElideMiddle, label->property(EXTRAINFO_SIZE).toInt());
        label->setText(elidedText);
    }
    else
    {
        label->setText(text);
    }

    label->setProperty(MESSAGE_TEXT, text);
}

void StalledIssueActionTitle::updateExtraInfoLayout()
{
    for(int row = 0; row < ui->extraInfoLayout->count(); ++row)
    {
        QLabel* infoLabel(nullptr);

        auto SizeAvailable(ui->generalContainer->width()
                           - (ui->extraInfoLayout->contentsMargins().left() + ui->extraInfoLayout->contentsMargins().right()));

        auto childLabels = ui->extraInfoLayout->itemAt(row)->widget()->findChildren<QLabel*>();
        foreach(auto label, childLabels)
        {
            if(label->property(EXTRAINFO_INFO).toBool())
            {
                if(label != childLabels.last())
                {
                    auto size = label->fontMetrics().width(label->property(MESSAGE_TEXT).toString());
                    SizeAvailable -= (size + 25);
                }
                else
                {
                    infoLabel = label;
                }
            }
            else
            {
                auto size = (10 + label->fontMetrics().width(label->text()));
                SizeAvailable -= size;
            }
        }

        if(infoLabel && SizeAvailable > 0)
        {
            infoLabel->setProperty(EXTRAINFO_SIZE, SizeAvailable);
            auto elidedText = infoLabel->fontMetrics().elidedText(infoLabel->property(MESSAGE_TEXT).toString(), Qt::ElideMiddle, SizeAvailable);
            infoLabel->setText(elidedText);
        }
    }
}

void StalledIssueActionTitle::setInfo(const QString &newPath, mega::MegaHandle handle)
{
    mPath = newPath;

    if(mIsCloud)
    {
        mNode.reset(handle != mega::INVALID_HANDLE ? MegaSyncApp->getMegaApi()->getNodeByHandle(handle)
                                                                             : MegaSyncApp->getMegaApi()->getNodeByPath(mPath.toUtf8().constData()));
    }
}

bool StalledIssueActionTitle::isRawInfoVisible() const
{
    return MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible();
}
