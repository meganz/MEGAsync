#include "MessageDialogData.h"

#include "megaapi.h"
#include "MegaApplication.h"

#include <QCoreApplication>
#include <QVariantMap>

namespace
{
const QLatin1String BUTTON_VARIANT_LIST_TEXT("text");
const QLatin1String BUTTON_VARIANT_LIST_TYPE("type");
const QLatin1String BUTTON_VARIANT_LIST_STYLE("style");
const QUrl IMAGE_OK(QStringLiteral("qrc:/images/qml/ok.png"));
const QUrl IMAGE_WARNING(QStringLiteral("qrc:/images/qml/warning.png"));
const QUrl IMAGE_QUESTION(QStringLiteral("qrc:/images/qml/question.png"));
const QUrl IMAGE_ERROR(QStringLiteral("qrc:/images/qml/error.png"));
}

// =================================================================================================
// MessageDialogButtonInfo
// =================================================================================================

MessageDialogButtonInfo::MessageDialogButtonInfo(const QString& buttonText,
                                                 QMessageBox::StandardButton buttonType):
    text(buttonText),
    type(buttonType)
{}

// =================================================================================================
// MessageDialogCheckboxInfo
// =================================================================================================

MessageDialogCheckboxInfo::MessageDialogCheckboxInfo(const QString& checkboxText,
                                                     bool checkboxChecked):
    text(checkboxText),
    checked(checkboxChecked)
{}

QString MessageDialogCheckboxInfo::getText() const
{
    return text;
}

bool MessageDialogCheckboxInfo::getChecked() const
{
    return checked;
}

// =================================================================================================
// MessageDialogTextInfo
// =================================================================================================

MessageDialogTextInfo::MessageDialogTextInfo(const QString& newText, TextFormat textFormat):
    text(newText),
    format(textFormat)
{}

QString MessageDialogTextInfo::getText() const
{
    return text;
}

MessageDialogTextInfo::TextFormat MessageDialogTextInfo::getFormat() const
{
    return format;
}

// =================================================================================================
// MessageBoxResult
// =================================================================================================

MessageBoxResult::MessageBoxResult():
    mButton(QMessageBox::StandardButton::NoButton),
    mChecked(false)
{}

void MessageBoxResult::setButton(QMessageBox::StandardButton button)
{
    mButton = button;
}

void MessageBoxResult::setChecked(bool checked)
{
    mChecked = checked;
}

QMessageBox::StandardButton MessageBoxResult::result() const
{
    return mButton;
}

bool MessageBoxResult::isChecked() const
{
    return mChecked;
}

// =================================================================================================
// MessageBoxInfo
// =================================================================================================

MessageBoxInfo::MessageBoxInfo():
    finishFunc(nullptr),
    parent(nullptr),
    dialogTitle(MegaSyncApp->getMEGAString()),
    titleText(QString()),
    descriptionText(QString()),
    buttons(QMessageBox::NoButton),
    defaultButton(QMessageBox::NoButton),
    textFormat(Qt::PlainText),
    imageUrl(QUrl()),
    enqueue(false),
    ignoreCloseAll(false),
    hideCloseButton(false),
    checkboxText(QString()),
    checkboxChecked(false)
{}

// =================================================================================================
// MessageDialogData
// =================================================================================================

MessageDialogData::MessageDialogData(Type type, MessageBoxInfo info, QObject* parent):
    QObject(parent),
    mType(type),
    mInfo(info),
    mResult(new MessageBoxResult)
{
    updateWidgetsByType();
    setButtons(mInfo.buttons, mInfo.defaultButton);
}

MessageDialogData::Type MessageDialogData::getType() const
{
    return mType.value();
}

QWidget* MessageDialogData::getParentWidget() const
{
    return mInfo.parent;
}

QString MessageDialogData::getTitle() const
{
    return mInfo.dialogTitle;
}

QUrl MessageDialogData::getImageUrl() const
{
    return mInfo.imageUrl;
}

MessageDialogTextInfo MessageDialogData::getTitleTextInfo() const
{
    return MessageDialogTextInfo(mInfo.titleText, getTextFormat());
}

MessageDialogTextInfo MessageDialogData::getDescriptionTextInfo() const
{
    return MessageDialogTextInfo(mInfo.descriptionText, getTextFormat());
}

bool MessageDialogData::enqueue() const
{
    return mInfo.enqueue;
}

bool MessageDialogData::ignoreCloseAll() const
{
    return mInfo.ignoreCloseAll;
}

MessageDialogCheckboxInfo MessageDialogData::getCheckbox() const
{
    return MessageDialogCheckboxInfo(mInfo.checkboxText, mInfo.checkboxChecked);
}

QPointer<MessageBoxResult> MessageDialogData::result() const
{
    return mResult;
}

QVariantList MessageDialogData::getButtons() const
{
    QVariantList list;
    for (auto it = mButtons.constBegin(); it != mButtons.constEnd(); ++it)
    {
        QVariantMap buttonData;
        buttonData[BUTTON_VARIANT_LIST_TEXT] = it.value().text;
        buttonData[BUTTON_VARIANT_LIST_TYPE] = static_cast<int>(it.value().type);
        buttonData[BUTTON_VARIANT_LIST_STYLE] = static_cast<int>(it.value().style);
        list.append(buttonData);
    }
    return list;
}

void MessageDialogData::buttonClicked(QMessageBox::StandardButton type)
{
    if (!mButtons.contains(type))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           QString::fromUtf8("MessageDialogData: button type %1 not found")
                               .arg(QString::number(type))
                               .toUtf8()
                               .constData());
        return;
    }

    mResult->setButton(type);
}

void MessageDialogData::setImageUrl(const QUrl& url)
{
    if (mInfo.imageUrl == url)
    {
        return;
    }

    mInfo.imageUrl = url;
    emit imageChanged();
}

std::function<void(QPointer<MessageBoxResult>)> MessageDialogData::getFinishFunction() const
{
    return mInfo.finishFunc;
}

void MessageDialogData::setCheckboxChecked(bool checked)
{
    if (mInfo.checkboxChecked == checked)
    {
        return;
    }

    mInfo.checkboxChecked = checked;
    mResult->setChecked(checked);
    emit checkboxChanged();
}

void MessageDialogData::setButtons(QMessageBox::StandardButtons buttons,
                                   QMessageBox::StandardButton defaultButton)
{
    if (buttons.testFlag(QMessageBox::StandardButton::NoButton))
    {
        return;
    }

    processButtonInfo(buttons, QMessageBox::StandardButton::Ok, tr("OK"));
    processButtonInfo(buttons, QMessageBox::StandardButton::Yes, tr("Yes"));
    processButtonInfo(buttons, QMessageBox::StandardButton::No, tr("No"));
    processButtonInfo(buttons, QMessageBox::StandardButton::Cancel, tr("Cancel"));

    if (mButtons.contains(defaultButton))
    {
        mButtons[defaultButton].style = MessageDialogButtonInfo::ButtonStyle::PRIMARY;
    }

    emit buttonsChanged();
}

void MessageDialogData::processButtonInfo(QMessageBox::StandardButtons buttons,
                                          QMessageBox::StandardButton type,
                                          const QString& defaultText)
{
    if (buttons.testFlag(type))
    {
        QString buttonText(mInfo.buttonsText.contains(type) ? mInfo.buttonsText[type] :
                                                              defaultText);
        mButtons.insert(type, MessageDialogButtonInfo(buttonText, type));
    }
}

void MessageDialogData::updateButtonsByDefault(QMessageBox::StandardButtons buttons,
                                               QMessageBox::StandardButton defaultButton)
{
    if (mInfo.buttons == QMessageBox::NoButton)
    {
        mInfo.buttons = buttons;
        mInfo.defaultButton = defaultButton;
    }
}

void MessageDialogData::updateWidgetsByType()
{
    QUrl imageUrl;

    // Set default values based on the type.
    switch (mType.value())
    {
        case Type::INFORMATION:
        {
            imageUrl = IMAGE_OK;
            updateButtonsByDefault(QMessageBox::StandardButton::Ok);
            break;
        }
        case Type::WARNING:
        {
            imageUrl = IMAGE_WARNING;
            updateButtonsByDefault(QMessageBox::StandardButton::Ok);
            break;
        }
        case Type::QUESTION:
        {
            imageUrl = IMAGE_QUESTION;
            updateButtonsByDefault(QMessageBox::StandardButton::Yes |
                                       QMessageBox::StandardButton::No,
                                   QMessageBox::StandardButton::No);
            break;
        }
        case Type::CRITICAL:
        {
            imageUrl = IMAGE_ERROR;
            updateButtonsByDefault(QMessageBox::StandardButton::Ok);
            break;
        }
        default:
        {
            break;
        }
    }

    if (mInfo.imageUrl.isEmpty())
    {
        setImageUrl(imageUrl);
    }
}

MessageDialogTextInfo::TextFormat MessageDialogData::getTextFormat() const
{
    switch (mInfo.textFormat)
    {
        case Qt::RichText:
        {
            return MessageDialogTextInfo::TextFormat::RICH;
        }
        case Qt::PlainText:
        {
            return MessageDialogTextInfo::TextFormat::PLAIN;
        }
        default:
        {
            mega::MegaApi::log(
                mega::MegaApi::LOG_LEVEL_WARNING,
                QString::fromUtf8("MessageDialogData: unsupported text format %1, using plain text")
                    .arg(QString::number(mInfo.textFormat))
                    .toUtf8()
                    .constData());
            return MessageDialogTextInfo::TextFormat::PLAIN;
        }
    }
}
