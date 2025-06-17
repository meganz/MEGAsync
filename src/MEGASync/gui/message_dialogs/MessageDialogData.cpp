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
const QLatin1String BUTTON_VARIANT_LIST_ICON("iconUrl");
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
// MessageDialogResult
// =================================================================================================

MessageDialogResult::MessageDialogResult(QObject* parent):
    QObject(parent),
    mButton(QMessageBox::StandardButton::NoButton),
    mChecked(false)
{}

void MessageDialogResult::setButton(QMessageBox::StandardButton button)
{
    mButton = button;
}

void MessageDialogResult::setChecked(bool checked)
{
    mChecked = checked;
}

QMessageBox::StandardButton MessageDialogResult::result() const
{
    return mButton;
}

bool MessageDialogResult::isChecked() const
{
    return mChecked;
}

// =================================================================================================
// MessageBoxInfo
// =================================================================================================

MessageDialogInfo::MessageDialogInfo():
    finishFunc(nullptr),
    parent(nullptr),
    buttons(QMessageBox::NoButton),
    defaultButton(QMessageBox::NoButton),
    textFormat(Qt::PlainText),
    enqueue(false),
    hideCloseButton(false),
    checkboxChecked(false),
    dialogTitle(MegaSyncApp->getMEGAString()),
    ignoreCloseAll(true)
{}

QString MessageDialogInfo::getDialogTitle() const
{
    return dialogTitle;
}

bool MessageDialogInfo::getIgnoreCloseAll() const
{
    return ignoreCloseAll;
}

void MessageDialogInfo::setIgnoreCloseAll(bool value)
{
    ignoreCloseAll = value;
}

// =================================================================================================
// MessageDialogData
// =================================================================================================

MessageDialogData::MessageDialogData(Type type, MessageDialogInfo info, QObject* parent):
    QObject(parent),
    mType(type),
    mInfo(info),
    mResult(new MessageDialogResult(this))
{
    updateWidgetsByType();
    buildButtons();
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
    return mInfo.getDialogTitle();
}

QUrl MessageDialogData::getImageUrl() const
{
    return mInfo.imageUrl;
}

MessageDialogTextInfo MessageDialogData::getTitleTextInfo() const
{
    if (mInfo.titleText.isEmpty() && !mInfo.descriptionText.isEmpty())
    {
        return MessageDialogTextInfo(mInfo.descriptionText, getTextFormat());
    }

    return MessageDialogTextInfo(mInfo.titleText, getTextFormat());
}

MessageDialogTextInfo MessageDialogData::getDescriptionTextInfo() const
{
    if (mInfo.titleText.isEmpty() && !mInfo.descriptionText.isEmpty())
    {
        return MessageDialogTextInfo(QString(), getTextFormat());
    }

    return MessageDialogTextInfo(mInfo.descriptionText, getTextFormat());
}

bool MessageDialogData::enqueue() const
{
    return mInfo.enqueue;
}

bool MessageDialogData::ignoreCloseAll() const
{
    return mInfo.getIgnoreCloseAll();
}

MessageDialogCheckboxInfo MessageDialogData::getCheckbox() const
{
    return MessageDialogCheckboxInfo(mInfo.checkboxText, mInfo.checkboxChecked);
}

QPointer<MessageDialogResult> MessageDialogData::result() const
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
        buttonData[BUTTON_VARIANT_LIST_ICON] = it.value().iconUrl;
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

    mResult->setChecked(mInfo.checkboxChecked);
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

std::function<void(QPointer<MessageDialogResult>)> MessageDialogData::getFinishFunction() const
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
    emit checkboxChanged();
}

void MessageDialogData::buildButtons()
{
    if (mInfo.buttons.testFlag(QMessageBox::StandardButton::NoButton))
    {
        return;
    }

    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::Ok,
                      QCoreApplication::translate("QDialogButtonBox", "&OK"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::Yes,
                      QApplication::translate("QDialogButtonBox", "&Yes"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::No,
                      QApplication::translate("QDialogButtonBox", "&No"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::Cancel,
                      QApplication::translate("QDialogButtonBox", "&Cancel"));

    if (mButtons.contains(mInfo.defaultButton))
    {
        mButtons[mInfo.defaultButton].style = MessageDialogButtonInfo::ButtonStyle::PRIMARY;
    }

    for (auto it = mInfo.buttonsIcons.constBegin(); it != mInfo.buttonsIcons.constEnd(); ++it)
    {
        if (mButtons.contains(it.key()))
        {
            mButtons[it.key()].iconUrl = it.value();
        }
    }

    emit buttonsChanged();
}

void MessageDialogData::processButtonInfo(QMessageBox::StandardButtons buttons,
                                          QMessageBox::StandardButton type,
                                          QString defaultText)
{
    if (buttons.testFlag(type))
    {
        // QML's Button does not interpret & as a mnemonic (shortcut) the way QWidgets
        // So to avoid changing the .ts file (and also because it is still in use by other classes)
        // we continue translating using the & but we remove it later
        if (defaultText.startsWith(QLatin1Char('&')))
        {
            defaultText.remove(0, 1);
        }

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
    }
    if (mInfo.defaultButton == QMessageBox::NoButton)
    {
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
