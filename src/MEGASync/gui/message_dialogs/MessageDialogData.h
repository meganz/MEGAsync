#ifndef MESSAGE_DIALOG_DATA_H
#define MESSAGE_DIALOG_DATA_H

#include <QIcon>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QUrl>

#include <functional>
#include <optional>

class MessageDialogComponent;

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
    MessageDialogButtonInfo(const QString& buttonText, QMessageBox::StandardButton buttonType);
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

struct MessageDialogTextInfo
{
    Q_GADGET

    Q_PROPERTY(QString text READ getText MEMBER text)
    Q_PROPERTY(TextFormat format READ getFormat MEMBER format)

public:
    enum class TextFormat
    {
        PLAIN = 0,
        RICH = 1,
    };
    Q_ENUM(TextFormat)

    QString text = QString();
    TextFormat format = TextFormat::PLAIN;

    MessageDialogTextInfo() = default;
    MessageDialogTextInfo(const QString& text, TextFormat format = TextFormat::PLAIN);

    QString getText() const;
    TextFormat getFormat() const;
};
Q_DECLARE_METATYPE(MessageDialogTextInfo)

class MessageDialogResult: public QObject
{
    Q_OBJECT

public:
    MessageDialogResult();
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
    QWidget* parent;
    QString titleText;
    QString descriptionText;
    QMessageBox::StandardButtons buttons;
    QMessageBox::StandardButton defaultButton;
    QMap<QMessageBox::StandardButton, QString> buttonsText;
    QMap<QMessageBox::StandardButton, QUrl> buttonsIcons;
    Qt::TextFormat textFormat;
    QUrl imageUrl;
    bool enqueue;
    bool ignoreCloseAll;
    bool hideCloseButton;
    QString checkboxText;
    bool checkboxChecked;

    QString getDialogTitle() const;

private:
    // Dialog title must always be the same for all message dialogs, in order to avoid unexpexted
    // changes in the dialog title this member is maintained as private.
    QString dialogTitle;
};

class MessageDialogData: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ getTitle CONSTANT)
    Q_PROPERTY(QUrl imageUrl READ getImageUrl NOTIFY imageChanged)
    Q_PROPERTY(MessageDialogTextInfo titleTextInfo READ getTitleTextInfo CONSTANT)
    Q_PROPERTY(MessageDialogTextInfo descriptionTextInfo READ getDescriptionTextInfo CONSTANT)
    Q_PROPERTY(QVariantList buttons READ getButtons NOTIFY buttonsChanged)
    Q_PROPERTY(MessageDialogCheckboxInfo checkbox READ getCheckbox NOTIFY checkboxChanged)

public:
    enum class Type
    {
        INFORMATION = 1,
        WARNING = 2,
        QUESTION = 3,
        CRITICAL = 4,
    };

    explicit MessageDialogData(Type type, MessageDialogInfo info, QObject* parent = nullptr);
    virtual ~MessageDialogData() = default;

    Type getType() const;
    QWidget* getParentWidget() const;
    QString getTitle() const;
    QUrl getImageUrl() const;
    MessageDialogTextInfo getTitleTextInfo() const;
    MessageDialogTextInfo getDescriptionTextInfo() const;
    QVariantList getButtons() const;
    std::function<void(QPointer<MessageDialogResult>)> getFinishFunction() const;
    bool enqueue() const;
    bool ignoreCloseAll() const;
    MessageDialogCheckboxInfo getCheckbox() const;

    QPointer<MessageDialogResult> result() const;

signals:
    void typeChanged();
    void imageChanged();
    void buttonsChanged();
    void checkboxChanged();

private:
    std::optional<Type> mType;
    MessageDialogInfo mInfo;
    QPointer<MessageDialogResult> mResult;
    QMap<QMessageBox::StandardButton, MessageDialogButtonInfo> mButtons;

    friend class MessageDialogComponent;

    void setCheckboxChecked(bool checked);
    void buttonClicked(QMessageBox::StandardButton type);

    void setImageUrl(const QUrl& url);
    void buildButtons();
    void processButtonInfo(QMessageBox::StandardButtons buttons,
                           QMessageBox::StandardButton type,
                           const QString& defaultText);
    void updateButtonsByDefault(QMessageBox::StandardButtons buttons,
                                QMessageBox::StandardButton defaultButton = QMessageBox::Ok);
    void updateWidgetsByType();
    MessageDialogTextInfo::TextFormat getTextFormat() const;
};

#endif // MESSAGE_DIALOG_DATA_H
