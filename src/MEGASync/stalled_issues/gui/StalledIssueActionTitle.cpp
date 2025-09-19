#include "StalledIssueActionTitle.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesUtilities.h"
#include "ThemeManager.h"
#include "TokenizableItems/TokenizableButtons.h"
#include "TokenizableItems/TokenPropertyNames.h"
#include "TokenParserWidgetManager.h"
#include "ui_StalledIssueActionTitle.h"
#include "Utilities.h"

#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSpacerItem>

const char* BUTTON_ID = "button_id";
const char* MAIN_BUTTON = "main";
const char* DISCARDED = "discarded";
const char* FAILED_BACKGROUND = "failed";
const char* DISABLE_BACKGROUND = "disable_background";
const char* MESSAGE_TEXT = "message_text";
const char* EXTRAINFO_INFO = "extrainfo_info";
const char* EXTRAINFO_SIZE = "extrainfo_size";

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
    ui->hoverIcon->hide();

    ui->backgroundWidget->setProperty(DISABLE_BACKGROUND, false);
    ui->backgroundWidget->setProperty(DISCARDED,false);
    ui->backgroundWidget->setProperty(FAILED_BACKGROUND, false);

    setAttribute(Qt::WA_StyledBackground, true);
    ui->backgroundWidget->setAttribute(Qt::WA_StyledBackground, true);
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

void StalledIssueActionTitle::setHTML(const QString& title,
                                      const QString& iconPath,
                                      const QString& iconToken)
{
    auto font(ui->titleLabel->font());
    font.setBold(false);
    ui->titleLabel->setFont(font);
    ui->titleLabel->setTextFormat(Qt::AutoText);

    setTitle(title, iconPath, iconToken);
}

void StalledIssueActionTitle::setTitle(const QString& title,
                                       const QString& iconPath,
                                       const QString& iconToken)
{
    updateSizeHints();

    ui->titleLabel->setText(title);

    mIconPath = iconPath;
    mIconToken = iconToken;

    updateIcon();
}

QString StalledIssueActionTitle::title() const
{
    return ui->titleLabel->toPlainText();
}

void StalledIssueActionTitle::setHyperLinkMode()
{
    ui->titleContainer->setCursor(Qt::PointingHandCursor);
    ui->titleContainer->installEventFilter(this);
    ui->titleContainer->setMouseTracking(true);
}

void StalledIssueActionTitle::addActionButton(const QIcon& icon,
                                              const QString& text,
                                              int id,
                                              bool mainButton,
                                              const QString& type)
{
    //Update existing buttons
    auto buttons = ui->actionContainer->findChildren<TokenizableButton*>();
    foreach(auto& button, buttons)
    {
        if(button->property(BUTTON_ID).toInt() == id)
        {
            button->setIcon(icon);
            button->setText(text);
            button->setProperty("type", type);
            button->setProperty(MAIN_BUTTON, mainButton);
            return;;
        }
    }

    auto button = new TokenizableButton(this);
    button->setIcon(icon);
    button->setText(text);
    button->setProperty(BUTTON_ID, id);
    button->setProperty(MAIN_BUTTON,mainButton);
    if (!type.isEmpty())
    {
        button->setProperty("type", type);
    }
    button->setProperty("dimension", QLatin1String("small"));
    button->setCursor(Qt::PointingHandCursor);

    connect(button,
            &TokenizableButton::clicked,
            this,
            [this]()
            {
                QApplication::postEvent(this,
                                        new QMouseEvent(QEvent::MouseButtonPress,
                                                        QPointF(),
                                                        Qt::LeftButton,
                                                        Qt::NoButton,
                                                        Qt::KeyboardModifier::AltModifier));
                qApp->processEvents();
                emit actionClicked(sender()->property(BUTTON_ID).toInt());
            });

    ui->actionLayout->addWidget(button, 0, Qt::AlignRight);

    if(!icon.isNull())
    {
        button->setIconSize(QSize(16, 16));

        if(text.isEmpty())
        {
            button->setFixedSize(QSize(24, 24));
        }
    }

    ui->actionContainer->show();
}

void StalledIssueActionTitle::setActionButtonVisibility(int id, bool state)
{
    bool allHidden(true);

    auto buttons = ui->actionContainer->findChildren<QPushButton*>();
    foreach(auto& button, buttons)
    {
        if(button->property(BUTTON_ID).toInt() == id)
        {
            button->setVisible(state);
            if(state)
            {
                setMessage(QString());
                allHidden = false;
            }
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
    else if(ui->actionContainer->isHidden())
    {
        ui->actionContainer->show();
    }
}

void StalledIssueActionTitle::showIcon()
{
    QFileInfo fileInfo(mPath);

    QString fileTypeIcon = StalledIssuesUtilities::getIcon(mIsFile, fileInfo);

    if(!fileTypeIcon.isNull())
    {
        ui->icon->setIcon(QIcon(fileTypeIcon));
        ui->icon->setIconSize(QSize(24, 24));
        ui->icon->show();
    }
}

void StalledIssueActionTitle::setMessage(const QString& message,
                                         const QString& pixmapName,
                                         const QString& iconToken,
                                         const QString& tooltip)
{
    updateSizeHints();
    ui->messageContainer->show();
    ui->messageContainer->installEventFilter(this);
    ui->messageContainer->setToolTip(tooltip);

    ui->iconLabel->clear();
    ui->iconLabel->setProperty(TOKEN_PROPERTIES::normalOff, iconToken);
    ui->iconLabel->setIcon(QIcon(pixmapName));

    ui->messageLabel->setText(ui->messageLabel->fontMetrics().elidedText(message, Qt::ElideMiddle, ui->contents->width()/3));
    ui->messageLabel->setProperty(MESSAGE_TEXT, message);
}

void StalledIssueActionTitle::addExtraInfo(AttributeType type, const QString& title, const QString& info, int level)
{
    ui->extraInfoContainer->show();

    auto titleLabel = new QLabel(title, this);
    titleLabel->setProperty("font-size", QLatin1String("caption"));

    auto infoLabel = new QLabel(info, this);
    infoLabel->setProperty(MESSAGE_TEXT, info);
    infoLabel->setProperty(EXTRAINFO_INFO, true);
    infoLabel->setProperty("font-size", QLatin1String("caption"));

    mTitleLabels[type] = titleLabel;
    mUpdateLabels[type] = infoLabel;

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
}

void StalledIssueActionTitle::setFailed(bool state, const QString& errorTooltip)
{
    setToolTip(errorTooltip);
    ui->backgroundWidget->setProperty(FAILED_BACKGROUND, state);
    setStyleSheet(styleSheet());
}

void StalledIssueActionTitle::setDisable(bool state)
{
    ui->backgroundWidget->setProperty(DISCARDED, state);
    ui->icon->setDisabled(state);
    setStyleSheet(styleSheet());
}

bool StalledIssueActionTitle::isSolved() const
{
    return ui->backgroundWidget->property(DISCARDED).toBool();
}

bool StalledIssueActionTitle::isFailed() const
{
    return ui->backgroundWidget->property(FAILED_BACKGROUND).toBool();
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
    else if(watched == ui->titleContainer)
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            StalledIssuesUtilities::openLink(mIsCloud, mPath);
        }
        else if(event->type() == QEvent::Enter)
        {
            ui->hoverIcon->show();
        }
        else if(event->type() == QEvent::Leave)
        {
            ui->hoverIcon->hide();
        }
    }

    return QWidget::eventFilter(watched, event);
}

bool StalledIssueActionTitle::updateUser(const QString& user, bool show)
{
    auto userLabel = mUpdateLabels.value(AttributeType::User);
    bool visible(userLabel && !userLabel->text().isEmpty());

    if(show)
    {
        auto titleString(tr("Upload by:"));
        auto userText = user.isEmpty() ? tr("Loading user…") : user;
        if(!userLabel)
        {
            addExtraInfo(AttributeType::User, titleString, userText, 1);
        }
        else
        {
            mTitleLabels[AttributeType::User]->setText(titleString);
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
    auto versionsLabel = mUpdateLabels.value(AttributeType::Versions);
    bool visible(versionsLabel && !versionsLabel->text().isEmpty());
    bool show(versions > 1);

    if(show)
    {
        auto titleString(tr("Versions:"));
        QString versionsText(QString::number(versions));
        if(!versionsLabel)
        {
            addExtraInfo(AttributeType::Versions, titleString, versionsText, 1);
        }
        else
        {
            mTitleLabels[AttributeType::Versions]->setText(titleString);
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

    auto titleString(tr("Size:"));
    auto sizeLabel = mUpdateLabels.value(AttributeType::Size);
    if(!sizeLabel)
    {
        addExtraInfo(AttributeType::Size, titleString, sizeText, 0);
    }
    else
    {
        mTitleLabels[AttributeType::Size]->setText(titleString);
        updateLabel(sizeLabel, sizeText);
    }

    showAttribute(AttributeType::Size);
}

void StalledIssueActionTitle::updateCRC(const QString& fp)
{
    auto rawValues = isRawInfoVisible();

    if(rawValues)
    {
        QString fpText = fp.isEmpty() ? QLatin1String("-") : fp;

        auto titleString(tr("CRC:"));
        auto fpLabel = mUpdateLabels.value(AttributeType::CRC);
        if(!fpLabel)
        {
            addExtraInfo(AttributeType::CRC, titleString, fpText, 0);
        }
        else
        {
            mTitleLabels[AttributeType::CRC]->setText(titleString);
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

    auto titleString(tr("Last modified:"));
    auto lastTimeLabel = mUpdateLabels.value(AttributeType::LastModified);
    if(!lastTimeLabel)
    {
        addExtraInfo(AttributeType::LastModified, titleString, timeString, 0);
    }
    else
    {
        mTitleLabels[AttributeType::LastModified]->setText(titleString);
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

    auto titleString(mIsCloud ? tr("Upload at:") : tr("Created at:"));

    auto createdTimeLabel = mUpdateLabels.value(AttributeType::CreatedTime);
    if(!createdTimeLabel)
    {
        addExtraInfo(AttributeType::CreatedTime, titleString, timeString, 0);
    }
    else
    {
        mTitleLabels[AttributeType::CreatedTime]->setText(titleString);
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

void StalledIssueActionTitle::updateIcon()
{
    if (!mIconPath.isEmpty())
    {
        ui->icon->clear();
        ui->icon->setIcon(QIcon(mIconPath));
        ui->icon->setIconSize(QSize(16, 16));
        ui->icon->setProperty(TOKEN_PROPERTIES::normalOff, mIconToken);
        ui->icon->show();
    }
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
