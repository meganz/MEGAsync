#include "IconTokenizer.h"

#include <QDebug>
#include <QWidget>
#include <QBitmap>
#include <QToolButton>

static const QString ButtonId = QString::fromUtf8("Button");
static const QString CustomId = QString::fromUtf8("Custom");

IconTokenizer::IconTokenizer(QObject* parent)
    : QObject{parent}
{ }

void IconTokenizer::process(QWidget* widget, const QString& mode, const QString& state, const ColorTokens& colorTokens,
                            const QString& targetElementId, const QString& targetElementProperty, const QString& tokenId)
{
    if (widget == nullptr || mode.isEmpty() || state.isEmpty() || colorTokens.empty()|| targetElementId.isEmpty() || targetElementProperty.isEmpty() || tokenId.isEmpty())
    {
        qWarning() << __func__ << " Error on function arguments :"
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
        qWarning() << __func__ << " Error children widget not found for  : " << targetElementId;

        return;
    }

    if (targetElementProperty == ButtonId)
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

        for (auto childWidget : widgets)
        {
            auto button = dynamic_cast<QAbstractButton*>(childWidget);
            if (button == nullptr)
            {
                qWarning() << __func__ << " Error dynamic cast failed for Widget* to QAbstractButton* : " << targetElementId;
                return;
            }

            QIcon buttonIcons = button->icon();
            if (buttonIcons.isNull())
            {
                qWarning() << __func__ << " Error button icon is null : " << targetElementId;
                return;
            }

            auto pixmap =
                buttonIcons.pixmap(button->iconSize(), iconMode.value(), iconState.value());
            if (pixmap.isNull())
            {
                qWarning() << __func__ << " Error default pixmap for icon is null : " << targetElementId;
                return;
            }

            if (!colorTokens.contains(tokenId))
            {
                qWarning() << __func__ << " Error token id not found : " << tokenId;
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
    else if (targetElementProperty == CustomId)
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

        for (auto childWidget: widgets)
        {
            QIcon icons = childWidget->property("icon").value<QIcon>();
            if (icons.isNull())
            {
                qWarning() << __func__ << " Error Custom Widget icon is null : " << targetElementId;
                return;
            }

            auto pixmap = icons.pixmap(childWidget->property("iconSize").toSize(),
                                       iconMode.value(),
                                       iconState.value());
            if (pixmap.isNull())
            {
                qWarning() << __func__
                           << " Error default pixmap for icon is null : " << targetElementId;
                return;
            }

            if (!colorTokens.contains(tokenId))
            {
                qWarning() << __func__ << " Error token id not found : " << tokenId;
                return;
            }

            QColor toColor(colorTokens.value(tokenId));

            auto tintedPixmap = changePixmapColor(pixmap, toColor);

            if (tintedPixmap.has_value())
            {
                icons.addPixmap(tintedPixmap.value(), iconMode.value(), iconState.value());
                childWidget->setProperty("icon", icons);
            }
        }
    }
}

std::optional<QIcon::Mode> IconTokenizer::getIconMode(const QString& mode)
{
    static const QHash<QString, QIcon::Mode> modeMap {
        {QString::fromUtf8("normal"), QIcon::Normal},
        {QString::fromUtf8("disabled"), QIcon::Disabled},
        {QString::fromUtf8("active"), QIcon::Active},
        {QString::fromUtf8("selected"), QIcon::Selected}
    };

    auto it = modeMap.find(mode);
    if (it != modeMap.end())
    {
        return it.value();
    }

    qWarning() << __func__ << " Error unknown icon mode: " << mode;
    return std::nullopt;
}

std::optional<QIcon::State> IconTokenizer::getIconState(const QString& state)
{
    static const QHash<QString, QIcon::State> stateMap {
        {QStringLiteral("on"), QIcon::On},
        {QStringLiteral("off"), QIcon::Off}
    };

    auto it = stateMap.find(state);
    if (it != stateMap.end())
    {
        return it.value();
    }

    qWarning() << __func__ << " Error unknown icon state: " << state;
    return std::nullopt;
}

std::optional<QPixmap> IconTokenizer::changePixmapColor(const QPixmap& pixmap, QColor toColor)
{
    if (pixmap.isNull())
    {
        qWarning() << __func__ << " Error pixmap argument is invalid";

        return std::nullopt;
    }

    QImage image = pixmap.toImage();
    if (image.isNull())
    {
        qWarning() << __func__ << " Error image from pixmap is invalid";

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
