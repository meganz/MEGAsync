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
}

// =================================================================================================
// MessageDialogButtonInfo
// =================================================================================================

MessageDialogButtonInfo::MessageDialogButtonInfo(const QString& buttonText,
                                                 QMessageBox::StandardButton buttonType,
                                                 ButtonStyle buttonStyle):
    text(buttonText),
    type(buttonType),
    style(buttonStyle)
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
// MessageDialogTextInfo (SNC-6567 Phase 3: QObject with NOTIFY signals)
// =================================================================================================

MessageDialogTextInfo::MessageDialogTextInfo(QObject* parent):
    QObject(parent)
{}

QString MessageDialogTextInfo::text() const
{
    return mText;
}

void MessageDialogTextInfo::setText(const QString& value)
{
    if (mText == value)
    {
        return;
    }
    mText = value;
    emit textChanged();
}

MessageDialogTextInfo::TextFormat MessageDialogTextInfo::format() const
{
    return mFormat;
}

void MessageDialogTextInfo::setFormat(TextFormat value)
{
    if (mFormat == value)
    {
        return;
    }
    mFormat = value;
    emit formatChanged();
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

// =================================================================================================
// MessageDialogData
// =================================================================================================

MessageDialogData::MessageDialogData(Type type, MessageDialogInfo info, QObject* parent):
    QObject(parent),
    mType(type),
    mInfo(info),
    mResult(new MessageDialogResult(this)),
    // SNC-6567 (Phase 3): Owned QObject children, lifetime tied to `this`.
    // Initial text values are set below after updateWidgetsByType()/
    // buildButtons() (some types swap title <-> description).
    mTitleTextInfo(new MessageDialogTextInfo(this)),
    mDescriptionTextInfo(new MessageDialogTextInfo(this))
{
    updateWidgetsByType();
    buildButtons();

    // Populate the text-info objects from mInfo, preserving the existing
    // behavior that promotes descriptionText to the title slot when titleText
    // is empty.
    const auto textFormat = getTextFormat();
    if (mInfo.titleText.isEmpty() && !mInfo.descriptionText.isEmpty())
    {
        mTitleTextInfo->setText(mInfo.descriptionText);
        mTitleTextInfo->setFormat(textFormat);
        mDescriptionTextInfo->setText(QString());
        mDescriptionTextInfo->setFormat(textFormat);
    }
    else
    {
        mTitleTextInfo->setText(mInfo.titleText);
        mTitleTextInfo->setFormat(textFormat);
        mDescriptionTextInfo->setText(mInfo.descriptionText);
        mDescriptionTextInfo->setFormat(textFormat);
    }
}

MessageDialogData::Type MessageDialogData::getType() const
{
    return mType;
}

QWidget* MessageDialogData::getParentDialog() const
{
    if (mInfo.parent)
    {
        auto isDialog = qobject_cast<QDialog*>(mInfo.parent) != nullptr;

        if (!isDialog)
        {
            QWidget* currentParent(mInfo.parent->parentWidget());
            QDialog* parentDialog(nullptr);
            while (!parentDialog && currentParent)
            {
                parentDialog = qobject_cast<QDialog*>(currentParent);
                currentParent = currentParent->parentWidget();
            }

            if (parentDialog)
            {
                return parentDialog;
            }
        }
    }

    return mInfo.parent;
}

QString MessageDialogData::getTitle() const
{
    return mInfo.getDialogTitle();
}

// SNC-6567 (Phase 3): Return the long-lived QObject children. The pointer is
// stable for the dialog's lifetime; their internal text/format can be mutated
// (e.g. on retranslation) and QML bindings refresh automatically via the
// NOTIFY signals.
MessageDialogTextInfo* MessageDialogData::getTitleTextInfo() const
{
    return mTitleTextInfo;
}

MessageDialogTextInfo* MessageDialogData::getDescriptionTextInfo() const
{
    return mDescriptionTextInfo;
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
                      MessageDialogButtonInfo::ButtonStyle::PRIMARY,
                      QCoreApplication::translate("QDialogButtonBox", "&OK"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::Yes,
                      MessageDialogButtonInfo::ButtonStyle::PRIMARY,
                      QApplication::translate("QDialogButtonBox", "&Yes"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::No,
                      MessageDialogButtonInfo::ButtonStyle::OUTLINE,
                      QApplication::translate("QDialogButtonBox", "&No"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::Cancel,
                      MessageDialogButtonInfo::ButtonStyle::OUTLINE,
                      QApplication::translate("QDialogButtonBox", "&Cancel"));
    processButtonInfo(mInfo.buttons,
                      QMessageBox::StandardButton::Close,
                      MessageDialogButtonInfo::ButtonStyle::PRIMARY,
                      tr("Close"));

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
                                          MessageDialogButtonInfo::ButtonStyle buttonStyle,
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

        mButtons.insert(type, MessageDialogButtonInfo(buttonText, type, buttonStyle));
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
    // Set default values based on the type.
    switch (mType)
    {
        case Type::SUCCESS:
        {
            updateButtonsByDefault(QMessageBox::StandardButton::Close);
            break;
        }
        case Type::INFORMATION:
        {
            updateButtonsByDefault(QMessageBox::StandardButton::Ok);
            break;
        }
        case Type::WARNING:
        {
            updateButtonsByDefault(QMessageBox::StandardButton::Ok);
            break;
        }
        case Type::QUESTION:
        {
            updateButtonsByDefault(QMessageBox::StandardButton::Yes |
                                       QMessageBox::StandardButton::No,
                                   QMessageBox::StandardButton::No);
            break;
        }
        case Type::CRITICAL:
        {
            updateButtonsByDefault(QMessageBox::StandardButton::Ok);
            break;
        }
        default:
        {
            break;
        }
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
