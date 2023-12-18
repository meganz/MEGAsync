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

StalledIssueActionTitle::StalledIssueActionTitle(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueActionTitle),
    mIsCloud(false),
    mIsFile(false)
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
    setStyleSheet(styleSheet());
}

void StalledIssueActionTitle::setTitle(const QString& title, const QPixmap& icon)
{
    updateSizeHints();

    ui->titleLabel->setText(title);
    if(!icon.isNull())
    {
        ui->icon->setFixedSize(icon.size());
        ui->icon->setPixmap(icon);
        ui->icon->show();
    }
}

QString StalledIssueActionTitle::title() const
{
    return ui->titleLabel->toPlainText();
}

void StalledIssueActionTitle::addActionButton(const QIcon& icon,const QString& text, int id, bool mainButton)
{
    auto button = new QPushButton(icon, text, this);

    button->setProperty(BUTTON_ID, id);
    button->setProperty(ONLY_ICON, false);
    button->setProperty(MAIN_BUTTON,mainButton);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked, this, [this]()
    {
        QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::AltModifier));
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

void StalledIssueActionTitle::setActionButtonInfo(const QIcon &icon, const QString &text, int id)
{
    auto buttons = ui->actionContainer->findChildren<QPushButton*>();
    foreach(auto& button, buttons)
    {
        if(button->property(BUTTON_ID).toInt() == id)
        {
            button->setIcon(icon);
            button->setText(text);
            break;
        }
    }
}

void StalledIssueActionTitle::showIcon()
{
    QFileInfo fileInfo(mPath);

    QIcon fileTypeIcon = StalledIssuesUtilities::getIcon(mIsFile, fileInfo, false);

    if(!fileTypeIcon.isNull())
    {
        ui->icon->setPixmap(fileTypeIcon.pixmap(ui->icon->size()));
        ui->icon->show();
    }
}

void StalledIssueActionTitle::setMessage(const QString& message, const QPixmap& pixmap)
{
    updateSizeHints();
    ui->messageContainer->show();
    ui->messageContainer->installEventFilter(this);

    if(!pixmap.isNull())
    {
        ui->iconLabel->setPixmap(pixmap);
    }

    ui->messageLabel->setText(ui->messageLabel->fontMetrics().elidedText(message, Qt::ElideMiddle, ui->contents->width()/3));
    ui->messageLabel->setProperty(MESSAGE_TEXT, message);
}

QLabel* StalledIssueActionTitle::addExtraInfo(const QString& title, const QString& info, int level)
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
}

bool StalledIssueActionTitle::isSolved() const
{
    return ui->backgroundWidget->property(DISCARDED).toBool();
}

void StalledIssueActionTitle::setIsCloud(bool state)
{
    mIsCloud = state;
}

bool StalledIssueActionTitle::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Resize)
    {
        if(watched == ui->generalContainer)
        {
            updateExtraInfoLayout();
        }
        else if(watched == ui->messageContainer)
        {
            auto width = ui->contents->width()/3;

            auto elidedText = ui->messageLabel->fontMetrics().elidedText(ui->messageLabel->property(MESSAGE_TEXT).toString(), Qt::ElideMiddle, width);
            ui->messageLabel->setText(elidedText);
        }
    }

    return QWidget::eventFilter(watched, event);
}

bool StalledIssueActionTitle::updateUser(const QString& user, bool show)
{
    auto& userLabel = mUpdateLabels[AttributeType::User];
    bool visible(userLabel && !userLabel->text().isEmpty());

    if(show)
    {
        auto userText = user.isEmpty() ? tr("Loading user…") : user;
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
        if(userLabel)
        {
            userLabel->setText(QString());
        }

        hideAttribute(AttributeType::User);
    }

    //As it is added to the second row, we need to know when it has been added/removed to modify the row height
    return show != visible;
}

bool StalledIssueActionTitle::updateVersionsCount(int versions)
{
    auto& versionsLabel = mUpdateLabels[AttributeType::Versions];
    bool visible(versionsLabel && !versionsLabel->text().isEmpty());
    bool show(versions > 1);

    if(show)
    {
        QString versionsText(QString::number(versions));
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
        if(versionsLabel)
        {
            versionsLabel->setText(QString());
        }

        hideAttribute(AttributeType::Versions);
    }


    //As it is added to the second row, we need to know when it has been added/removed to modify the row height
    return show != visible;
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

void StalledIssueActionTitle::updateCRC(const QString& fp)
{
    auto rawValues = isRawInfoVisible();

    if(rawValues)
    {
        QString fpText = fp.isEmpty() ? tr("-") : fp;

        auto& fpLabel = mUpdateLabels[AttributeType::CRC];
        if(!fpLabel)
        {
            fpLabel = addExtraInfo(tr("CRC:"), fpText, 0);
        }
        else
        {
            updateLabel(fpLabel, fpText);
        }

        showAttribute(AttributeType::CRC);
    }
    else
    {
        hideAttribute(AttributeType::CRC);
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
                    auto size = label->fontMetrics().horizontalAdvance(label->property(MESSAGE_TEXT).toString());
                    SizeAvailable -= (size + 25);
                }
                else
                {
                    infoLabel = label;
                }
            }
            else
            {
                auto size = (10 + label->fontMetrics().horizontalAdvance(label->text()));
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

void StalledIssueActionTitle::updateSizeHints()
{
    layout()->activate();

    ui->backgroundWidget->layout()->activate();
    ui->actionContainer->updateGeometry();

    ui->generalContainer->layout()->activate();
    ui->generalContainer->updateGeometry();

    ui->contents->layout()->activate();
    ui->contents->updateGeometry();

    ui->messageContainer->layout()->activate();
    ui->messageContainer->updateGeometry();

    ui->messageLabel->updateGeometry();

    ui->titleContainer->layout()->activate();
    ui->titleContainer->updateGeometry();

    ui->titleLabel->updateGeometry();
}

void StalledIssueActionTitle::setIsFile(bool newIsFile)
{
    mIsFile = newIsFile;
}

void StalledIssueActionTitle::setInfo(const QString &newPath, mega::MegaHandle handle)
{
    if(mPath != newPath)
    {
        mPath = newPath;
    }

    if(mIsCloud && (!mNode || (mNode->getHandle() != handle)))
    {
        mNode.reset(handle != mega::INVALID_HANDLE ? MegaSyncApp->getMegaApi()->getNodeByHandle(handle)
                                                                             : MegaSyncApp->getMegaApi()->getNodeByPath(mPath.toUtf8().constData()));
    }
}

bool StalledIssueActionTitle::isRawInfoVisible() const
{
    return MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible();
}
