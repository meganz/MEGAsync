#ifndef ICON_TOKENIZER_H
#define ICON_TOKENIZER_H

#include <QObject>
#include <QIcon>

#include <optional>

class IconTokenizer : public QObject
{
    Q_OBJECT

    using ColorTokens = QMap<QString, QString>;

public:
    static void process(QWidget* widget,
                        const QString& mode,
                        const QString& state,
                        const ColorTokens& colorTokens,
                        const QString& targetElementId,
                        const QString& targetElementProperty,
                        const QString& tokenId);

    static void tokenizeButtonIcon(QWidget* widget,
                                   const QIcon::Mode& mode,
                                   const QIcon::State& state,
                                   const QColor& toColor);

    static std::optional<QPixmap> changePixmapColor(const QPixmap& pixmap, QColor toColor);

private:
    explicit IconTokenizer(QObject* parent = nullptr);

    static std::optional<QIcon::Mode> getIconMode(const QString& mode);
    static std::optional<QIcon::State> getIconState(const QString& state);
};

#endif
