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
    explicit IconTokenizer(QObject *parent = nullptr);
    void process(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens, const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId);

private:
    void changePixmapColor(QPixmap& pixmap, QColor toColor);

    std::optional<QIcon::Mode> getIconMode(const QString& mode);
    std::optional<QIcon::State> getIconState(const QString& state);
};

#endif
