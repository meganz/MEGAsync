#ifndef MESSAGE_DIALOG_DATA_H
#define MESSAGE_DIALOG_DATA_H

#include <QIcon>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QUrl>

#include <functional>

class MessageDialogComponent;
class QmlDialog;

struct MessageDialogButtonInfo
{
    Q_GADGET

public:
    enum class ButtonStyle
    {
        OUTLINE = 0,
        PRIMARY = 1,
        SECONDARY = 2,
        LINK = 3,
        TEXT = 4
    };
    Q_ENUM(ButtonStyle)

    QString text = QString();
    QUrl iconUrl = QUrl();
    QMessageBox::StandardButton type = QMessageBox::StandardButton::NoButton;
    ButtonStyle style = ButtonStyle::OUTLINE;

    MessageDialogButtonInfo() = default;
    MessageDialogButtonInfo(const QString& buttonText,
                            QMessageBox::StandardButton buttonType,
                            ButtonStyle buttonStyle);
};

struct MessageDialogCheckboxInfo
{
    Q_GADGET

    Q_PROPERTY(QString text READ getText MEMBER text)
    Q_PROPERTY(bool checked READ getChecked MEMBER checked)

public:
    QString text = QString();
    bool checked = false;

    MessageDialogCheckboxInfo() = default;
    MessageDialogCheckboxInfo(const QString& checkboxText, bool checkboxChecked = false);

    QString getText() const;
    bool getChecked() const;
};
Q_DECLARE_METATYPE(MessageDialogCheckboxInfo)

// SNC-6567 (Phase 3): MessageDialogTextInfo is a proper QObject (not a Q_GADGET
// value type) with NOTIFY signals on its text/format properties. This lets QML
// bindings on `messageDialogDataAccess.titleTextInfo.text` refresh
// automatically when the underlying C++ value changes — enabling dynamic
// content updates (e.g. retranslation of an open dialog) and removing the
// fragile single-shot refresh cascade that the old Q_GADGET CONSTANT pattern
// depended on.
class MessageDialogTextInfo: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(TextFormat format READ format WRITE setFormat NOTIFY formatChanged)

public:
    enum class TextFormat
    {
        PLAIN = 0,
        RICH = 1,
    };
    Q_ENUM(TextFormat)

    explicit MessageDialogTextInfo(QObject* parent = nullptr);

    QString text() const;
    void setText(const QString& value);

    TextFormat format() const;
    void setFormat(TextFormat value);

signals:
    void textChanged();
    void formatChanged();

private:
    QString mText;
    TextFormat mFormat = TextFormat::PLAIN;
};

class MessageDialogResult: public QObject
{
    Q_OBJECT

public:
    MessageDialogResult(QObject* parent);

    virtual ~MessageDialogResult() = default;

    void setButton(QMessageBox::StandardButton button);
    void setChecked(bool checked);

    QMessageBox::StandardButton result() const;
    bool isChecked() const;

private:
    QMessageBox::StandardButton mButton;
    bool mChecked;
};

struct MessageDialogInfo
{
    MessageDialogInfo();

    std::function<void(QPointer<MessageDialogResult>)> finishFunc;
    QPointer<QWidget> parent;
    QPointer<QmlDialog> parentQml;
    QString titleText;
    QString descriptionText;
    QMessageBox::StandardButtons buttons;
    QMessageBox::StandardButton defaultButton;
    QMap<QMessageBox::StandardButton, QString> buttonsText;
    QMap<QMessageBox::StandardButton, QUrl> buttonsIcons;
    Qt::TextFormat textFormat;
    bool enqueue;
    bool hideCloseButton;
    QString checkboxText;
    bool checkboxChecked;

    QString getDialogTitle() const;
    bool getIgnoreCloseAll() const;

private:
    // Dialog title must always be the same for all message dialogs, in order to avoid unexpexted
    // changes in the dialog title this member is maintained as private.
    QString dialogTitle;

    // All messageboxes are closed when clicked a button
    bool ignoreCloseAll;
};

class MessageDialogData: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ getTitle CONSTANT)
    Q_PROPERTY(Type type READ getType NOTIFY typeChanged)
    // SNC-6567 (Phase 3): returning pointers to long-lived MessageDialogTextInfo
    // objects (owned by this MessageDialogData via Qt parent-child ownership).
    // The pointer itself does not change, so CONSTANT is correct here; the
    // text/format inside the pointee have their own NOTIFY signals so QML
    // bindings stay reactive to value changes.
    Q_PROPERTY(MessageDialogTextInfo* titleTextInfo READ getTitleTextInfo CONSTANT)
    Q_PROPERTY(MessageDialogTextInfo* descriptionTextInfo READ getDescriptionTextInfo CONSTANT)
    Q_PROPERTY(QVariantList buttons READ getButtons NOTIFY buttonsChanged)
    Q_PROPERTY(MessageDialogCheckboxInfo checkbox READ getCheckbox NOTIFY checkboxChanged)

public:
    enum class Type
    {
        SUCCESS = 0,
        INFORMATION = 1,
        WARNING = 2,
        QUESTION = 3,
        CRITICAL = 4,
    };
    Q_ENUM(Type)

    explicit MessageDialogData(Type type, MessageDialogInfo info, QObject* parent = nullptr);
    virtual ~MessageDialogData() = default;

    Type getType() const;
    QWidget* getParentDialog() const;
    QString getTitle() const;
    MessageDialogTextInfo* getTitleTextInfo() const;
    MessageDialogTextInfo* getDescriptionTextInfo() const;
    QVariantList getButtons() const;
    std::function<void(QPointer<MessageDialogResult>)> getFinishFunction() const;
    bool enqueue() const;
    bool ignoreCloseAll() const;
    MessageDialogCheckboxInfo getCheckbox() const;

    QPointer<MessageDialogResult> result() const;

signals:
    void typeChanged();
    void buttonsChanged();
    void checkboxChanged();

private:
    Type mType;
    MessageDialogInfo mInfo;
    QPointer<MessageDialogResult> mResult;
    QMap<QMessageBox::StandardButton, MessageDialogButtonInfo> mButtons;
    // SNC-6567 (Phase 3): MessageDialogTextInfo are now QObject children of
    // this MessageDialogData. Pointers stay valid for the dialog's lifetime;
    // their internal text/format can be mutated via setText()/setFormat() with
    // QML bindings auto-refreshing via the NOTIFY signals.
    MessageDialogTextInfo* mTitleTextInfo;
    MessageDialogTextInfo* mDescriptionTextInfo;

    friend class MessageDialogComponent;

    void setCheckboxChecked(bool checked);
    void buttonClicked(QMessageBox::StandardButton type);

    void setImageUrl(const QUrl& url);
    void setImageSize(const QSize& size);
    void buildButtons();
    void processButtonInfo(QMessageBox::StandardButtons buttons,
                           QMessageBox::StandardButton type,
                           MessageDialogButtonInfo::ButtonStyle buttonStyle,
                           QString defaultText = QString());
    void updateButtonsByDefault(QMessageBox::StandardButtons buttons,
                                QMessageBox::StandardButton defaultButton = QMessageBox::Ok);
    void updateWidgetsByType();
    MessageDialogTextInfo::TextFormat getTextFormat() const;
};

#endif // MESSAGE_DIALOG_DATA_H
