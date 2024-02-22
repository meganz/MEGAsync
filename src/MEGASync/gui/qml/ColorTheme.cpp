#include "ColorTheme.h"

#include "QmlTheme.h"

#include "megaapi.h"

#include <QQmlComponent>
#include <QDir>

#include <iostream>

static const QString ColorStyleFilePath = QString::fromUtf8("qrc:/common/themes/%1/Colors.qml");

ColorTheme::ColorTheme(const QmlTheme* const theme, QQmlEngine *engine, QObject *parent):
    QObject(parent),
    mEngine(engine),
    mTheme(theme)
{
    assert(mTheme != nullptr);

    mCurrentTheme = mTheme->getTheme();

    connect(mTheme, &QmlTheme::themeChanged, this, &ColorTheme::onThemeChanged);

    loadThemes();
}

void ColorTheme::onThemeChanged(QString theme)
{    
    if (!theme.isEmpty() && mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        emit valueChanged();
    }
}

void ColorTheme::loadThemes()
{
    for (const auto& theme: mTheme->getThemes())
    {
        QQmlComponent qmlComponent(mEngine);
        QString colorStyleFile = ColorStyleFilePath.arg(theme);
        qmlComponent.loadUrl(QUrl(colorStyleFile));

        if (qmlComponent.isError())
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error loading color style file : %1")
                .arg(colorStyleFile).toUtf8().constData());
        }
        else
        {
            mThemesMap[theme] = qmlComponent.create();
        }
    }
}

QString ColorTheme::getValue(QString tokenId)
{
    QString returnValue;

    if(mThemesMap.contains(mCurrentTheme))
    {
        returnValue = mThemesMap[mCurrentTheme]->property(tokenId.toStdString().c_str()).toString();
    }

    return returnValue;
}

ColorTheme::~ColorTheme()
{
    qDeleteAll(mThemesMap);
}

/*
 * Property binding style functions.
*/
QString ColorTheme::borderInteractive()
{
    return getValue(QString::fromUtf8("borderInteractive"));
}

QString ColorTheme::borderStrong()
{
    return getValue(QString::fromUtf8("borderStrong"));
}

QString ColorTheme::borderStrongSelected()
{
    return getValue(QString::fromUtf8("borderStrongSelected"));
}

QString ColorTheme::borderSubtle()
{
    return getValue(QString::fromUtf8("borderSubtle"));
}

QString ColorTheme::borderSubtleSelected()
{
    return getValue(QString::fromUtf8("borderSubtleSelected"));
}

QString ColorTheme::borderDisabled()
{
    return getValue(QString::fromUtf8("borderDisabled"));
}

QString ColorTheme::linkPrimary()
{
    return getValue(QString::fromUtf8("linkPrimary"));
}

QString ColorTheme::linkInverse()
{
    return getValue(QString::fromUtf8("linkInverse"));
}

QString ColorTheme::linkVisited()
{
    return getValue(QString::fromUtf8("linkVisited"));
}

QString ColorTheme::buttonBrand()
{
    return getValue(QString::fromUtf8("buttonBrand"));
}

QString ColorTheme::buttonBrandHover()
{
    return getValue(QString::fromUtf8("buttonBrandHover"));
}

QString ColorTheme::buttonBrandPressed()
{
    return getValue(QString::fromUtf8("buttonBrandPressed"));
}

QString ColorTheme::buttonPrimary()
{
    return getValue(QString::fromUtf8("buttonPrimary"));
}

QString ColorTheme::buttonPrimaryHover()
{
    return getValue(QString::fromUtf8("buttonPrimaryHover"));
}

QString ColorTheme::buttonPrimaryPressed()
{
    return getValue(QString::fromUtf8("buttonPrimaryPressed"));
}

QString ColorTheme::buttonOutline()
{
    return getValue(QString::fromUtf8("buttonOutline"));
}

QString ColorTheme::buttonOutlineHover()
{
    return getValue(QString::fromUtf8("buttonOutlineHover"));
}

QString ColorTheme::buttonOutlineBackgroundHover()
{
    return getValue(QString::fromUtf8("buttonOutlineBackgroundHover"));
}

QString ColorTheme::buttonOutlinePressed()
{
    return getValue(QString::fromUtf8("buttonOutlinePressed"));
}

QString ColorTheme::buttonSecondary()
{
    return getValue(QString::fromUtf8("buttonSecondary"));
}

QString ColorTheme::buttonSecondaryHover()
{
    return getValue(QString::fromUtf8("buttonSecondaryHover"));
}

QString ColorTheme::buttonSecondaryPressed()
{
    return getValue(QString::fromUtf8("buttonSecondaryPressed"));
}

QString ColorTheme::buttonError()
{
    return getValue(QString::fromUtf8("buttonError"));
}

QString ColorTheme::buttonErrorHover()
{
    return getValue(QString::fromUtf8("buttonErrorHover"));
}

QString ColorTheme::buttonErrorPressed()
{
    return getValue(QString::fromUtf8("buttonErrorPressed"));
}

QString ColorTheme::buttonDisabled()
{
    return getValue(QString::fromUtf8("buttonDisabled"));
}

QString ColorTheme::iconButton()
{
    return getValue(QString::fromUtf8("iconButton"));
}

QString ColorTheme::iconButtonHover()
{
    return getValue(QString::fromUtf8("iconButtonHover"));
}

QString ColorTheme::iconButtonPressed()
{
    return getValue(QString::fromUtf8("iconButtonPressed"));
}

QString ColorTheme::iconButtonPressedBackground()
{
    return getValue(QString::fromUtf8("iconButtonPressedBackground"));
}

QString ColorTheme::iconButtonDisabled()
{
    return getValue(QString::fromUtf8("iconButtonDisabled"));
}

QString ColorTheme::focus()
{
    return getValue(QString::fromUtf8("focus"));
}

QString ColorTheme::pageBackground()
{
    return getValue(QString::fromUtf8("pageBackground"));
}

QString ColorTheme::surface1()
{
    return getValue(QString::fromUtf8("surface1"));
}

QString ColorTheme::surface2()
{
    return getValue(QString::fromUtf8("surface2"));
}

QString ColorTheme::surface3()
{
    return getValue(QString::fromUtf8("surface3"));
}

QString ColorTheme::backgroundInverse()
{
    return getValue(QString::fromUtf8("backgroundInverse"));
}

QString ColorTheme::textPrimary()
{
    return getValue(QString::fromUtf8("textPrimary"));
}

QString ColorTheme::textSecondary()
{
    return getValue(QString::fromUtf8("textSecondary"));
}

QString ColorTheme::textAccent()
{
    return getValue(QString::fromUtf8("textAccent"));
}

QString ColorTheme::textPlaceholder()
{
    return getValue(QString::fromUtf8("textPlaceholder"));
}

QString ColorTheme::textInverseAccent()
{
    return getValue(QString::fromUtf8("textInverseAccent"));
}

QString ColorTheme::textOnColor()
{
    return getValue(QString::fromUtf8("textOnColor"));
}

QString ColorTheme::textOnColorDisabled()
{
    return getValue(QString::fromUtf8("textOnColorDisabled"));
}

QString ColorTheme::textError()
{
    return getValue(QString::fromUtf8("textError"));
}

QString ColorTheme::textSuccess()
{
    return getValue(QString::fromUtf8("textSuccess"));
}

QString ColorTheme::textInfo()
{
    return getValue(QString::fromUtf8("textInfo"));
}

QString ColorTheme::textWarning()
{
    return getValue(QString::fromUtf8("textWarning"));
}

QString ColorTheme::textInverse()
{
    return getValue(QString::fromUtf8("textInverse"));
}

QString ColorTheme::textDisabled()
{
    return getValue(QString::fromUtf8("textDisabled"));
}

QString ColorTheme::iconPrimary()
{
    return getValue(QString::fromUtf8("iconPrimary"));
}

QString ColorTheme::iconSecondary()
{
    return getValue(QString::fromUtf8("iconSecondary"));
}

QString ColorTheme::iconAccent()
{
    return getValue(QString::fromUtf8("iconAccent"));
}

QString ColorTheme::iconInverseAccent()
{
    return getValue(QString::fromUtf8("iconInverseAccent"));
}

QString ColorTheme::iconOnColor()
{
    return getValue(QString::fromUtf8("iconOnColor"));
}

QString ColorTheme::iconOnColorDisabled()
{
    return getValue(QString::fromUtf8("iconOnColorDisabled"));
}

QString ColorTheme::iconInverse()
{
    return getValue(QString::fromUtf8("iconInverse"));
}

QString ColorTheme::iconPlaceholder()
{
    return getValue(QString::fromUtf8("iconPlaceholder"));
}

QString ColorTheme::iconDisabled()
{
    return getValue(QString::fromUtf8("iconDisabled"));
}

QString ColorTheme::supportSuccess()
{
    return getValue(QString::fromUtf8("supportSuccess"));
}

QString ColorTheme::supportWarning()
{
    return getValue(QString::fromUtf8("supportWarning"));
}

QString ColorTheme::supportError()
{
    return getValue(QString::fromUtf8("supportError"));
}

QString ColorTheme::supportInfo()
{
    return getValue(QString::fromUtf8("supportInfo"));
}

QString ColorTheme::selectionControl()
{
    return getValue(QString::fromUtf8("selectionControl"));
}

QString ColorTheme::notificationSuccess()
{
    return getValue(QString::fromUtf8("notificationSuccess"));
}

QString ColorTheme::notificationWarning()
{
    return getValue(QString::fromUtf8("notificationWarning"));
}

QString ColorTheme::notificationError()
{
    return getValue(QString::fromUtf8("notificationError"));
}

QString ColorTheme::notificationInfo()
{
    return getValue(QString::fromUtf8("notificationInfo"));
}

QString ColorTheme::interactive()
{
    return getValue(QString::fromUtf8("interactive"));
}

QString ColorTheme::indicatorBackground()
{
    return getValue(QString::fromUtf8("indicatorBackground"));
}

QString ColorTheme::indicatorPink()
{
    return getValue(QString::fromUtf8("indicatorPink"));
}

QString ColorTheme::indicatorYellow()
{
    return getValue(QString::fromUtf8("indicatorYellow"));
}

QString ColorTheme::indicatorGreen()
{
    return getValue(QString::fromUtf8("indicatorGreen"));
}

QString ColorTheme::indicatorBlue()
{
    return getValue(QString::fromUtf8("indicatorBlue"));
}

QString ColorTheme::indicatorIndigo()
{
    return getValue(QString::fromUtf8("indicatorIndigo"));
}

QString ColorTheme::indicatorMagenta()
{
    return getValue(QString::fromUtf8("indicatorMagenta"));
}

QString ColorTheme::indicatorOrange()
{
    return getValue(QString::fromUtf8("indicatorOrange"));
}

QString ColorTheme::toastBackground()
{
    return getValue(QString::fromUtf8("toastBackground"));
}

QString ColorTheme::divider()
{
    return getValue(QString::fromUtf8("divider"));
}

QString ColorTheme::gradientRedTop()
{
    return getValue(QString::fromUtf8("gradientRedTop"));
}

QString ColorTheme::gradientRedBottom()
{
    return getValue(QString::fromUtf8("gradientRedBottom"));
}

QString ColorTheme::gradientGreyTop()
{
    return getValue(QString::fromUtf8("gradientGreyTop"));
}

QString ColorTheme::gradientGreyBottom()
{
    return getValue(QString::fromUtf8("gradientGreyBottom"));
}

QString ColorTheme::gradientPinkTop()
{
    return getValue(QString::fromUtf8("gradientPinkTop"));
}

QString ColorTheme::gradientPinkBottom()
{
    return getValue(QString::fromUtf8("gradientPinkBottom"));
}

QString ColorTheme::gradientYellowTop()
{
    return getValue(QString::fromUtf8("gradientYellowTop"));
}

QString ColorTheme::gradientYellowBottom()
{
    return getValue(QString::fromUtf8("gradientYellowBottom"));
}

QString ColorTheme::gradientGreenTop()
{
    return getValue(QString::fromUtf8("gradientGreenTop"));
}

QString ColorTheme::gradientGreenBottom()
{
    return getValue(QString::fromUtf8("gradientGreenBottom"));
}

QString ColorTheme::gradientIndigoTop()
{
    return getValue(QString::fromUtf8("gradientIndigoTop"));
}

QString ColorTheme::gradientIndigoBottom()
{
    return getValue(QString::fromUtf8("gradientIndigoBottom"));
}

QString ColorTheme::gradientMagentaTop()
{
    return getValue(QString::fromUtf8("gradientMagentaTop"));
}

QString ColorTheme::gradientMagentaBottom()
{
    return getValue(QString::fromUtf8("gradientMagentaBottom"));
}

QString ColorTheme::gradientOrangeTop()
{
    return getValue(QString::fromUtf8("gradientOrangeTop"));
}

QString ColorTheme::gradientOrangeBottom()
{
    return getValue(QString::fromUtf8("gradientOrangeBottom"));
}

QString ColorTheme::gradientBlueTop()
{
    return getValue(QString::fromUtf8("gradientBlueTop"));
}

QString ColorTheme::gradientBlueBottom()
{
    return getValue(QString::fromUtf8("gradientBlueBottom"));
}

QString ColorTheme::gradientContrastTop()
{
    return getValue(QString::fromUtf8("gradientContrastTop"));
}

QString ColorTheme::gradientContrastBottom()
{
    return getValue(QString::fromUtf8("gradientContrastBottom"));
}
