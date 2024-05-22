#include "IconTokenizer.h"

#include <QDebug>
#include <QWidget>
#include <QBitmap>
#include <QToolButton>

static const QString ToolButtonId = "ToolButton";

IconTokenizer::IconTokenizer(QObject* parent)
    : QObject{parent}
{ }

void IconTokenizer::process(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens,
                            const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId)
{
    if (widget == nullptr|| mode.isEmpty() || state.isEmpty() || colorTokens.empty()|| targetElementId.isEmpty() || targetElementProperty.isEmpty() || tokenId.isEmpty())
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

        auto button = dynamic_cast<QToolButton*>(widgets.constFirst());
        if (button == nullptr)
        {
            qDebug() << __func__ << " Error dynamic cast failed for Widget* to QToolButton* : " << targetElementId;
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

        changePixmapColor(pixmap, toColor);

        buttonIcons.addPixmap(pixmap, iconMode.value(), iconState.value());

        button->setIcon(buttonIcons);
    }
}

std::optional<QIcon::Mode> IconTokenizer::getIconMode(const QString& mode)
{
    std::optional<QIcon::Mode> iconMode;

    if (mode == "normal")
    {
        iconMode = QIcon::Normal;
    }
    else if (mode == "disabled")
    {
        iconMode = QIcon::Disabled;
    }
    else if (mode == "active")
    {
        iconMode =  QIcon::Active;
    }
    else if (mode == "selected")
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

    if (state == "on")
    {
        iconState = QIcon::On;
    }
    else if (state == "off")
    {
        iconState = QIcon::Off;
    }
    else
    {
        qDebug() << __func__ << " Error unknown icon state : " << state;
    }

    return iconState;
}

void IconTokenizer::changePixmapColor(QPixmap& pixmap, QColor toColor)
{
    if (pixmap.isNull())
    {
        return;
    }

    QImage image = pixmap.toImage();
    if (image.isNull())
    {
        return;
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

    pixmap = QPixmap::fromImage(image);
}
