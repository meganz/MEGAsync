#include "IconTokenizer.h"

#include "TokenizedIcon.h"

#include <QBitmap>
#include <QDebug>
#include <QToolButton>
#include <QWidget>

static const QString ButtonId = QString::fromUtf8("Button");
static const QString CustomId = QString::fromUtf8("Custom");

IconTokenizer::IconTokenizer(QObject* parent)
    : QObject{parent}
{ }

void IconTokenizer::process(QWidget* widget,
                            const QString& mode,
                            const QString& state,
                            const QString& targetElementId,
                            const QString& targetElementProperty,
                            const QString& tokenId)
{
    if (widget == nullptr || mode.isEmpty() || state.isEmpty() || targetElementId.isEmpty() ||
        targetElementProperty.isEmpty() || tokenId.isEmpty())
    {
        qWarning() << __func__ << " Error on function arguments :"
                   << "\n widget is nullptr : " << QVariant(widget == nullptr).toString()
                   << "\n mode is empty : " << QVariant(mode.isEmpty()).toString()
                   << "\n state is empty : " << QVariant(state.isEmpty()).toString()
                   << "\n targetElementId is empty : "
                   << QVariant(targetElementId.isEmpty()).toString()
                   << "\n targetElementProperty is empty : "
                   << QVariant(targetElementProperty.isEmpty()).toString()
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

        for (auto& childWidget: widgets)
        {
            tokenizeButtonIcon(childWidget, iconMode.value(), iconState.value(), tokenId);
        }
    }
}

void IconTokenizer::tokenizeButtonIcon(QWidget* widget,
                                       const QIcon::Mode& mode,
                                       const QIcon::State& state,
                                       const QString& token)
{
    auto button = dynamic_cast<QAbstractButton*>(widget);
    if (button == nullptr)
    {
        qWarning() << __func__ << " Error dynamic cast failed for Widget* to QAbstractButton* : "
                   << widget->objectName();
        return;
    }

    TokenizedIcon::addPixmap(button, mode, state, token);
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
