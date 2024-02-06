#include "ColorTheme.h"

#include "megaapi.h"

#include <QQmlComponent>
#include <QDir>

#include <iostream>

static const QString ColorStyleFilePath = QString::fromUtf8("qrc:/common/themes/colors/%1/Styles.qml");

ColorTheme::ColorTheme(QQmlEngine *engine, QObject *parent):
    QObject{parent},
    mEngine{engine}
{
    init();

    loadThemes();
}

void ColorTheme::init()
{
    // @jsubi.
    // TODO : get theme list.
    // TODO : get current theme.
    // TODO : set connection to capture theme changed event.

    // snipet to check theme change from light to dark.
    /*
    static QTimer timer;
    timer.start(10000);
    connect(&timer, &QTimer::timeout, this, [this](){
        onThemeChanged(QLatin1String("dark"));
    });
    */

    mCurrentTheme = QString().fromUtf8("light");
    mThemes << QString().fromUtf8("dark") << QString().fromUtf8("light");
}

void ColorTheme::onThemeChanged(QString theme)
{
    if (theme != mCurrentTheme)
    {
        mCurrentTheme = theme;

        emit valueChanged();
    }
}

void ColorTheme::loadThemes()
{
    for (const auto& theme: qAsConst(mThemes))
    {
        QQmlComponent qmlComponent(mEngine);
        QString colorStyleFile = ColorStyleFilePath.arg(theme);
        qmlComponent.loadUrl(QUrl(colorStyleFile));

        if (qmlComponent.isError())
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error loading color style file : %1").arg(colorStyleFile).toUtf8().constData());
        }
        else
        {
            mThemesMap[theme] = qmlComponent.create();
        }
    }
}

QString ColorTheme::getValue(const char* tokenId)
{
    QString returnValue;

    if(mThemes.contains(mCurrentTheme))
    {
        returnValue = mThemesMap[mCurrentTheme]->property(tokenId).toString();
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
    return getValue("borderInteractive");
}

QString ColorTheme::borderStrong()
{
    return getValue("borderStrong");
}

QString ColorTheme::borderStrongSelected()
{
    return getValue("borderStrongSelected");
}

QString ColorTheme::borderSubtle()
{
    return getValue("borderSubtle");
}

QString ColorTheme::borderSubtleSelected()
{
    return getValue("borderSubtleSelected");
}

QString ColorTheme::borderDisabled()
{
    return getValue("borderDisabled");
}

QString ColorTheme::linkPrimary()
{
    return getValue("linkPrimary");
}

QString ColorTheme::linkInverse()
{
    return getValue("linkInverse");
}

QString ColorTheme::linkVisited()
{
    return getValue("linkVisited");
}

QString ColorTheme::buttonBrand()
{
    return getValue("buttonBrand");
}

QString ColorTheme::buttonBrandHover()
{
    return getValue("buttonBrandHover");
}

QString ColorTheme::buttonBrandPressed()
{
    return getValue("buttonBrandPressed");
}

QString ColorTheme::buttonPrimary()
{
    return getValue("buttonPrimary");
}

QString ColorTheme::buttonPrimaryHover()
{
    return getValue("buttonPrimaryHover");
}

QString ColorTheme::buttonPrimaryPressed()
{
    return getValue("buttonPrimaryPressed");
}

QString ColorTheme::buttonOutline()
{
    return getValue("buttonOutline");
}

QString ColorTheme::buttonOutlineHover()
{
    return getValue("buttonOutlineHover");
}

QString ColorTheme::buttonOutlineBackgroundHover()
{
    return getValue("buttonOutlineBackgroundHover");
}

QString ColorTheme::buttonOutlinePressed()
{
    return getValue("buttonOutlinePressed");
}

QString ColorTheme::buttonSecondary()
{
    return getValue("buttonSecondary");
}

QString ColorTheme::buttonSecondaryHover()
{
    return getValue("buttonSecondaryHover");
}

QString ColorTheme::buttonSecondaryPressed()
{
    return getValue("buttonSecondaryPressed");
}

QString ColorTheme::buttonError()
{
    return getValue("buttonError");
}

QString ColorTheme::buttonErrorHover()
{
    return getValue("buttonErrorHover");
}

QString ColorTheme::buttonErrorPressed()
{
    return getValue("buttonErrorPressed");
}

QString ColorTheme::buttonDisabled()
{
    return getValue("buttonDisabled");
}

QString ColorTheme::iconButton()
{
    return getValue("iconButton");
}

QString ColorTheme::iconButtonHover()
{
    return getValue("iconButtonHover");
}

QString ColorTheme::iconButtonPressed()
{
    return getValue("iconButtonPressed");
}

QString ColorTheme::iconButtonPressedBackground()
{
    return getValue("iconButtonPressedBackground");
}

QString ColorTheme::iconButtonDisabled()
{
    return getValue("iconButtonDisabled");
}

QString ColorTheme::focus()
{
    return getValue("focus");
}

QString ColorTheme::pageBackground()
{
    return getValue("pageBackground");
}

QString ColorTheme::surface1()
{
    return getValue("surface1");
}

QString ColorTheme::surface2()
{
    return getValue("surface2");
}

QString ColorTheme::surface3()
{
    return getValue("surface3");
}

QString ColorTheme::backgroundInverse()
{
    return getValue("backgroundInverse");
}

QString ColorTheme::textPrimary()
{
    return getValue("textPrimary");
}

QString ColorTheme::textSecondary()
{
    return getValue("textSecondary");
}

QString ColorTheme::textAccent()
{
    return getValue("textAccent");
}

QString ColorTheme::textPlaceholder()
{
    return getValue("textPlaceholder");
}

QString ColorTheme::textInverseAccent()
{
    return getValue("textInverseAccent");
}

QString ColorTheme::textOnColor()
{
    return getValue("textOnColor");
}

QString ColorTheme::textOnColorDisabled()
{
    return getValue("textOnColorDisabled");
}

QString ColorTheme::textError()
{
    return getValue("textError");
}

QString ColorTheme::textSuccess()
{
    return getValue("textSuccess");
}

QString ColorTheme::textInfo()
{
    return getValue("textInfo");
}

QString ColorTheme::textWarning()
{
    return getValue("textWarning");
}

QString ColorTheme::textInverse()
{
    return getValue("textInverse");
}

QString ColorTheme::textDisabled()
{
    return getValue("textDisabled");
}

QString ColorTheme::iconPrimary()
{
    return getValue("iconPrimary");
}

QString ColorTheme::iconSecondary()
{
    return getValue("iconSecondary");
}

QString ColorTheme::iconAccent()
{
    return getValue("iconAccent");
}

QString ColorTheme::iconInverseAccent()
{
    return getValue("iconInverseAccent");
}

QString ColorTheme::iconOnColor()
{
    return getValue("iconOnColor");
}

QString ColorTheme::iconOnColorDisabled()
{
    return getValue("iconOnColorDisabled");
}

QString ColorTheme::iconInverse()
{
    return getValue("iconInverse");
}

QString ColorTheme::iconPlaceholder()
{
    return getValue("iconPlaceholder");
}

QString ColorTheme::iconDisabled()
{
    return getValue("iconDisabled");
}

QString ColorTheme::supportSuccess()
{
    return getValue("supportSuccess");
}

QString ColorTheme::supportWarning()
{
    return getValue("supportWarning");
}

QString ColorTheme::supportError()
{
    return getValue("supportError");
}

QString ColorTheme::supportInfo()
{
    return getValue("supportInfo");
}

QString ColorTheme::selectionControl()
{
    return getValue("selectionControl");
}

QString ColorTheme::notificationSuccess()
{
    return getValue("notificationSuccess");
}

QString ColorTheme::notificationWarning()
{
    return getValue("notificationWarning");
}

QString ColorTheme::notificationError()
{
    return getValue("notificationError");
}

QString ColorTheme::notificationInfo()
{
    return getValue("notificationInfo");
}

QString ColorTheme::interactive()
{
    return getValue("interactive");
}

QString ColorTheme::indicatorBackground()
{
    return getValue("indicatorBackground");
}

QString ColorTheme::indicatorPink()
{
    return getValue("indicatorPink");
}

QString ColorTheme::indicatorYellow()
{
    return getValue("indicatorYellow");
}

QString ColorTheme::indicatorGreen()
{
    return getValue("indicatorGreen");
}

QString ColorTheme::indicatorBlue()
{
    return getValue("indicatorBlue");
}

QString ColorTheme::indicatorIndigo()
{
    return getValue("indicatorIndigo");
}

QString ColorTheme::indicatorMagenta()
{
    return getValue("indicatorMagenta");
}

QString ColorTheme::indicatorOrange()
{
    return getValue("indicatorOrange");
}

QString ColorTheme::toastBackground()
{
    return getValue("toastBackground");
}

QString ColorTheme::divider()
{
    return getValue("divider");
}

QString ColorTheme::gradientRedTop()
{
    return getValue("gradientRedTop");
}

QString ColorTheme::gradientRedBottom()
{
    return getValue("gradientRedBottom");
}

QString ColorTheme::gradientGreyTop()
{
    return getValue("gradientGreyTop");
}

QString ColorTheme::gradientGreyBottom()
{
    return getValue("gradientGreyBottom");
}

QString ColorTheme::gradientPinkTop()
{
    return getValue("gradientPinkTop");
}

QString ColorTheme::gradientPinkBottom()
{
    return getValue("gradientPinkBottom");
}

QString ColorTheme::gradientYellowTop()
{
    return getValue("gradientYellowTop");
}

QString ColorTheme::gradientYellowBottom()
{
    return getValue("gradientYellowBottom");
}

QString ColorTheme::gradientGreenTop()
{
    return getValue("gradientGreenTop");
}

QString ColorTheme::gradientGreenBottom()
{
    return getValue("gradientGreenBottom");
}

QString ColorTheme::gradientIndigoTop()
{
    return getValue("gradientIndigoTop");
}

QString ColorTheme::gradientIndigoBottom()
{
    return getValue("gradientIndigoBottom");
}

QString ColorTheme::gradientMagentaTop()
{
    return getValue("gradientMagentaTop");
}

QString ColorTheme::gradientMagentaBottom()
{
    return getValue("gradientMagentaBottom");
}

QString ColorTheme::gradientOrangeTop()
{
    return getValue("gradientOrangeTop");
}

QString ColorTheme::gradientOrangeBottom()
{
    return getValue("gradientOrangeBottom");
}

QString ColorTheme::gradientBlueTop()
{
    return getValue("gradientBlueTop");
}

QString ColorTheme::gradientBlueBottom()
{
    return getValue("gradientBlueBottom");
}

QString ColorTheme::gradientContrastTop()
{
    return getValue("gradientContrastTop");
}

QString ColorTheme::gradientContrastBottom()
{
    return getValue("gradientContrastBottom");
}
