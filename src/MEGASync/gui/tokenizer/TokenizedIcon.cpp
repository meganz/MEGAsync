#include "TokenizedIcon.h"

#include "ThemeManager.h"
#include "tokenizer/IconTokenizer.h"
#include "TokenParserWidgetManager.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QVariant>
#include <QWindow>

const char* TokenizedIcon::TOKEN_ICON_PROPERTY = "toke_icon_property";

TokenizedIcon::TokenizedIcon(QAbstractButton* button):
    QObject(button),
    mButton(button),
    mThemeAppearance(ThemeManager::instance()->currentColorScheme())
{
    qRegisterMetaType<TokenizedIcon*>("TokenizedIcon*");

    performReset();

    mButton->setProperty(TOKEN_ICON_PROPERTY, QVariant::fromValue<TokenizedIcon*>(this));
}

TokenizedIcon* TokenizedIcon::getTokenizedIcon(QAbstractButton* button)
{
    TokenizedIcon* icon(nullptr);

    if (button)
    {
        icon = button->property(TokenizedIcon::TOKEN_ICON_PROPERTY).value<TokenizedIcon*>();
        if (!icon)
        {
            icon = new TokenizedIcon(button);
        }
    }

    return icon;
}

void TokenizedIcon::addPixmap(QAbstractButton* button,
                              QIcon::Mode mode,
                              QIcon::State state,
                              const QString& token)
{
    auto tokenizedIcon = getTokenizedIcon(button);
    if (tokenizedIcon)
    {
        tokenizedIcon->performAddPixmap(mode, state, token);
    }
}

void TokenizedIcon::reset(QAbstractButton* button)
{
    auto tokenizedIcon = getTokenizedIcon(button);
    if (tokenizedIcon)
    {
        tokenizedIcon->performReset();
    }
}

void TokenizedIcon::performAddPixmap(QIcon::Mode mode, QIcon::State state, const QString& token)
{
    auto currentThemeAppearance = ThemeManager::instance()->currentColorScheme();
    if (mThemeAppearance != currentThemeAppearance)
    {
        mThemeAppearance = currentThemeAppearance;
        mPixmapsByToken.clear();
    }

    // Check if it is an pixmap update
    auto previousTokenPixmapSize = mTokenizedIcon.pixmap(mButton->iconSize(), mode, state).size();
    auto pixmapsByToken = mPixmapsByToken.equal_range(token);
    for (auto it = pixmapsByToken.first; it != pixmapsByToken.second; ++it)
    {
        if ((*it).token == token && (*it).pixm.size() == previousTokenPixmapSize &&
            (*it).mode == mode && (*it).state == state)
        {
            addPixmapToIcon((*it).pixm, mode, state);
            mButton->setIcon(mTokenizedIcon);
            return;
        }
    }

    QColor toColor(TokenParserWidgetManager::instance()->getColor(token));
    if (!toColor.isValid())
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           QString::fromUtf8("Token color %1 not found for button %2")
                               .arg(token, mButton->objectName())
                               .toUtf8()
                               .constData());
    }

    auto untokenPixmap = mOriginalIcon.pixmap(mButton->iconSize(), mode, state);
    auto tokenizedPixmapOpt = IconTokenizer::changePixmapColor(untokenPixmap, toColor);
    auto tokenizedPixmap(tokenizedPixmapOpt.value_or(QPixmap()));
    if (!tokenizedPixmap.isNull())
    {
        PixmapInfo info;
        info.mode = mode;
        info.state = state;
        info.pixm = tokenizedPixmap;
        info.token = token;
        mPixmapsByToken.insert(token, info);

        addPixmapToIcon(tokenizedPixmap, mode, state);
        mButton->setIcon(mTokenizedIcon);
    }
}

void TokenizedIcon::performReset()
{
    mPixmapsByToken.clear();
    mOriginalIcon = mButton->icon();
}

void TokenizedIcon::addPixmapToIcon(const QPixmap& pix, QIcon::Mode mode, QIcon::State state)
{
    mTokenizedIcon.addPixmap(pix, mode, state);
    // // For us Normal and Active are the same
    if (mode == QIcon::Mode::Normal)
    {
        mTokenizedIcon.addPixmap(pix, QIcon::Mode::Active, state);
    }
    else if (mode == QIcon::Mode::Active)
    {
        mTokenizedIcon.addPixmap(pix, QIcon::Mode::Normal, state);
    }
}
