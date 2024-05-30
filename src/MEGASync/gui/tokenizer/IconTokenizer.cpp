#include "IconTokenizer.h"

#include <QDebug>
#include <QWidget>
#include <QBitmap>
#include <QToolButton>

static const QString ToolButtonId = QString::fromUtf8("Button");

IconTokenizer::IconTokenizer(QObject* parent)
    : QObject{parent}
{ }

void IconTokenizer::process(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens,
                            const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId)
{
    if (widget == nullptr || mode.isEmpty() || state.isEmpty() || colorTokens.empty()|| targetElementId.isEmpty() || targetElementProperty.isEmpty() || tokenId.isEmpty())
    {
        qDebug() << __func__ << " Error on function arguments :"
                 << "\n widget is nullptr : " << QVariant(widget == nullptr).toString()
                 << "\n mode is empty : " << QVariant(mode.isEmpty()).toString()
                 << "\n state is empty : " << QVariant(state.isEmpty()).toString()
                 << "\n colorTokens are empty : " << QVariant(colorTokens.isEmpty()).toString()
                 << "\n targetElementId is empty : " << QVariant(targetElementId.isEmpty()).toString()
                 << "\n targetElementProperty is empty : " << QVariant(targetElementProperty.isEmpty()).toString()
                 << "\n tokenId is empty : " << QVariant(tokenId.isEmpty()).toString();

        return;
    }

    auto widgets = widget->findChildren<QWidget*>(targetElementId);
    if (widgets.isEmpty())
    {
        qDebug() << __func__ << " Error children widget not found for  : " << targetElementId;

        return;
    }

    if (targetElementProperty == ToolButtonId)
    {
        auto iconState = getIconState(state);
        if (!iconState.has_value())
        {
            // error log created on getIconState
            return;
        }

        auto iconMode = getIconMode(mode);
        if (!iconMode.has_value())
        {
            // error log created on getIconMode
            return;
        }

        auto button = dynamic_cast<QAbstractButton*>(widgets.constFirst());
        if (button == nullptr)
        {
            qDebug() << __func__ << " Error dynamic cast failed for Widget* to QAbstractButton* : " << targetElementId;
            return;
        }

        QIcon buttonIcons = button->icon();
        if (buttonIcons.isNull())
        {
            qDebug() << __func__ << " Error button icon is null : " << targetElementId;
            return;
        }

        auto pixmap = buttonIcons.pixmap(button->iconSize());
        if (pixmap.isNull())
        {
            qDebug() << __func__ << " Error default pixmap for icon is null : " << targetElementId;
            return;
        }

        if (!colorTokens.contains(tokenId))
        {
            qDebug() << __func__ << " Error token id not found : " << tokenId;
            return;
        }

        QColor toColor(colorTokens.value(tokenId));

        auto tintedPixmap = changePixmapColor(pixmap, toColor);

        if (tintedPixmap.has_value())
        {
            buttonIcons.addPixmap(tintedPixmap.value(), iconMode.value(), iconState.value());
            button->setIcon(buttonIcons);
        }
    }
}

std::optional<QIcon::Mode> IconTokenizer::getIconMode(const QString& mode)
{
    std::optional<QIcon::Mode> iconMode;

    if (mode == QString::fromUtf8("normal"))
    {
        iconMode = QIcon::Normal;
    }
    else if (mode == QString::fromUtf8("disabled"))
    {
        iconMode = QIcon::Disabled;
    }
    else if (mode == QString::fromUtf8("active"))
    {
        iconMode =  QIcon::Active;
    }
    else if (mode == QString::fromUtf8("selected"))
    {
        iconMode = QIcon::Selected;
    }
    else
    {
        qDebug() << __func__ << " Error unknown icon mode : " << mode;
    }

    return iconMode;
}

std::optional<QIcon::State> IconTokenizer::getIconState(const QString& state)
{
    std::optional<QIcon::State> iconState;

    if (state == QString::fromUtf8("on"))
    {
        iconState = QIcon::On;
    }
    else if (state == QString::fromUtf8("off"))
    {
        iconState = QIcon::Off;
    }
    else
    {
        qDebug() << __func__ << " Error unknown icon state : " << state;
    }

    return iconState;
}

std::optional<QPixmap> IconTokenizer::changePixmapColor(const QPixmap& pixmap, QColor toColor)
{
    if (pixmap.isNull())
    {
        qDebug() << __func__ << " Error pixmap argument is invalid";

        return std::nullopt;
    }

    QImage image = pixmap.toImage();
    if (image.isNull())
    {
        qDebug() << __func__ << " Error image from pixmap is invalid";

        return std::nullopt;
    }

    /*
     * we are using the requested color for every pixel on the image
     * only the alpha channel is preserved.
    */
    for(auto widthIndex = 0; widthIndex < image.width(); ++widthIndex)
    {
        for(auto heightIndex = 0; heightIndex < image.height(); ++heightIndex)
        {
            QColor color = image.pixelColor(widthIndex, heightIndex);
            toColor.setAlpha(color.alpha());
            image.setPixelColor(widthIndex, heightIndex, toColor);
        }
    }

    return QPixmap::fromImage(image);
}
